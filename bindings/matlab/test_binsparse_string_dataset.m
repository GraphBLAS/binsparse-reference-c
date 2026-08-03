function test_binsparse_string_dataset()
%TEST_BINSPARSE_STRING_DATASET round-trip tests for the HDF5 text datasets
%
% Checks that binsparse_write_string_dataset and binsparse_read_string_dataset
% preserve both the contents and the MATLAB class of a text component: a char
% matrix is stored fixed-length and a cellstr variable-length, so the two are
% told apart by the HDF5 string datatype alone.

% SPDX-FileCopyrightText: 2026 Binsparse Developers
%
% SPDX-License-Identifier: BSD-3-Clause

fprintf('=== Testing Binsparse string datasets ===\n\n');

if exist('binsparse_write_string_dataset', 'file') ~= 3 || ...
        exist('binsparse_read_string_dataset', 'file') ~= 3
    error('test_binsparse_string_dataset:MissingMex', ...
          'The string dataset MEX functions are not on the path');
end

% Each case is {description, value}.  The value must survive isequal.
% An empty cellstr element comes back as the 1-by-0 char that sstextread
% produces, not as the 0-by-0 char that '' denotes: an HDF5 string has a
% length but no MATLAB shape, and 1-by-0 is the row that sscellstring accepts.
cases = {
    'char matrix, ragged rows blank padded', char({'alpha'; 'be'; 'gamma!'})
    'char matrix, every row full width',     ['abcd'; 'efgh'; 'ijkl']
    'char matrix, one row',                  'a single row'
    'char matrix, blanks on every row',      char({'ab   '; 'cd   '})
    'char matrix, leading blanks kept',      char({'  indented'; 'flush'})
    'char matrix, one row of blanks',        char({'text'; '    '})
    'char matrix, non-ASCII Latin-1',        char({['Jo' 227 'o Pessoa']; ['Bras' 237 'lia']})
    'char matrix, replacement characters',   char({[65533 65533 'x']; 'plain'})
    'char matrix, single column',            char({'a'; 'b'; 'c'})
    'cellstr, ragged',                       {'alpha'; 'be'; 'gamma!'}
    'cellstr, one element',                  {'only'}
    'cellstr, trailing blanks are content',  {'ab   '; 'cd'}
    'cellstr, empty element',                {'first'; char(zeros(1, 0)); 'third'}
    'cellstr, non-ASCII',                    {['Jo' 227 'o']; ['Bras' 237 'lia']}
    'cellstr, long strings',                 {repmat('x', 1, 5000); 'short'}
};

tmpdir = tempname;
mkdir(tmpdir);
cleanup = onCleanup(@() rmdir(tmpdir, 's'));

passed = 0;
failed = 0;
for level = [0 9]
    fprintf('-- compression level %d\n', level);
    for k = 1:size(cases, 1)
        description = cases{k, 1};
        value = cases{k, 2};
        filename = fullfile(tmpdir, sprintf('case_%d_%d.h5', level, k));
        try
            binsparse_write_string_dataset(filename, 'text', value, level);
            actual = binsparse_read_string_dataset(filename, '/text');
            check_equal(actual, value, description);
            fprintf('  PASS  %s\n', description);
            passed = passed + 1;
        catch me
            fprintf('  FAIL  %s: %s\n', description, me.message);
            failed = failed + 1;
        end
    end
end

% The class must come from the datatype, not from the shape: a one-element
% cellstr and a one-row char matrix hold the same text but must not be
% confused with one another.
filename = fullfile(tmpdir, 'classes.h5');
binsparse_write_string_dataset(filename, 'as_char', 'only');
binsparse_write_string_dataset(filename, 'as_cell', {'only'});
try
    if ~ischar(binsparse_read_string_dataset(filename, '/as_char')) || ...
            ~iscellstr(binsparse_read_string_dataset(filename, '/as_cell'))
        error('test:ClassNotPreserved', 'class was not preserved');
    end
    fprintf('  PASS  one-row char and one-element cellstr stay distinct\n');
    passed = passed + 1;
catch me
    fprintf('  FAIL  one-row char vs one-element cellstr: %s\n', me.message);
    failed = failed + 1;
end

% A char matrix should not pay for the variable-length global heap, which no
% dataset filter reaches.  A wide, mostly blank matrix must compress.
filename = fullfile(tmpdir, 'compressed.h5');
wide = repmat(['padded row' blanks(500)], 400, 1);
binsparse_write_string_dataset(filename, 'text', wide, 9);
info = dir(filename);
if isequal(binsparse_read_string_dataset(filename, '/text'), wide) && ...
        info.bytes < numel(wide) / 10
    fprintf('  PASS  blank padding compresses (%d bytes for %d characters)\n', ...
            info.bytes, numel(wide));
    passed = passed + 1;
else
    fprintf('  FAIL  blank padding did not compress (%d bytes)\n', info.bytes);
    failed = failed + 1;
end

% Compression is chosen on the total size, not the element count, so a single
% very wide row is compressed too.
filename = fullfile(tmpdir, 'one_row.h5');
one_row = [repmat('abcdefgh', 1, 20000) blanks(20000)];
binsparse_write_string_dataset(filename, 'text', one_row, 9);
info = dir(filename);
if isequal(binsparse_read_string_dataset(filename, '/text'), one_row) && ...
        info.bytes < numel(one_row) / 10
    fprintf('  PASS  a single wide row compresses (%d bytes for %d characters)\n', ...
            info.bytes, numel(one_row));
    passed = passed + 1;
else
    fprintf('  FAIL  a single wide row did not compress (%d bytes for %d)\n', ...
            info.bytes, numel(one_row));
    failed = failed + 1;
end

% Below the threshold no chunk index is created, and the data still reads back.
filename = fullfile(tmpdir, 'tiny.h5');
tiny = ['ab'; 'cd'];
binsparse_write_string_dataset(filename, 'text', tiny, 9);
if isequal(binsparse_read_string_dataset(filename, '/text'), tiny)
    fprintf('  PASS  a dataset below the compression threshold round-trips\n');
    passed = passed + 1;
else
    fprintf('  FAIL  a dataset below the compression threshold changed\n');
    failed = failed + 1;
end

fprintf('\n%d passed, %d failed\n', passed, failed);
if failed > 0
    error('test_binsparse_string_dataset:Failed', ...
          '%d string dataset test(s) failed', failed);
end
fprintf('=== All string dataset tests passed ===\n');

end

function check_equal(actual, expected, description)
if ~strcmp(class(actual), class(expected))
    error('test:ClassMismatch', 'expected %s, got %s (%s)', ...
          class(expected), class(actual), description);
end
if ~isequal(size(actual), size(expected))
    error('test:SizeMismatch', 'expected %s, got %s (%s)', ...
          mat2str(size(expected)), mat2str(size(actual)), description);
end
if ~isequal(actual, expected)
    error('test:ValueMismatch', 'contents differ (%s)', description);
end
end
