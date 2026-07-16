function test_binsparse_roundtrip_dir(root_dir, temp_dir)
% TEST_BINSPARSE_ROUNDTRIP_DIR - Round-trip binsparse files in a directory.
%
% This function scans a directory (recursively) for .h5 files, reads each
% with binsparse_read, writes to a temporary file with binsparse_write, then
% reads back and checks for equivalence.

% SPDX-FileCopyrightText: 2024 Binsparse Developers
%
% SPDX-License-Identifier: BSD-3-Clause

if nargin < 1 || isempty(root_dir)
    error('Usage: test_binsparse_roundtrip_dir(root_dir, [temp_dir])');
end

if ~isfolder(root_dir)
    error('Directory not found: %s', root_dir);
end

if ~exist('binsparse_read', 'file')
    error('binsparse_read MEX function not found. Build it first.');
end

if ~exist('binsparse_write', 'file')
    error('binsparse_write MEX function not found. Build it first.');
end

if nargin < 2 || isempty(temp_dir)
    temp_dir = '';
end

if ~isempty(temp_dir) && ~isfolder(temp_dir)
    error('Temp directory not found: %s', temp_dir);
end

files = list_h5_files(root_dir);
if isempty(files)
    fprintf('No .h5 files found under %s\n', root_dir);
    return;
end

fprintf('Found %d .h5 files under %s\n', numel(files), root_dir);
failures = 0;

for idx = 1:numel(files)
    file_path = files{idx};
    fprintf('\n[%d/%d] %s\n', idx, numel(files), file_path);

    try
        matrix = binsparse_read(file_path);

        if isempty(temp_dir)
            temp_file = [tempname(), '.bsp.h5'];
        else
            temp_file = [tempname(temp_dir), '.bsp.h5'];
        end
        cleanup = onCleanup(@() cleanup_temp_file(temp_file));

        binsparse_write(temp_file, matrix);
        roundtrip = binsparse_read(temp_file);

        [ok, message] = compare_binsparse_structs(matrix, roundtrip);
        if ok
            fprintf('  OK\n');
        else
            failures = failures + 1;
            fprintf('  MISMATCH: %s\n', message);
        end

        clear cleanup;
    catch ME
        failures = failures + 1;
        fprintf('  ERROR: %s\n', ME.message);
    end
end

if failures == 0
    fprintf('\nAll %d files passed round-trip checks.\n', numel(files));
else
    fprintf('\n%d of %d files failed round-trip checks.\n', failures, numel(files));
end

end

function files = list_h5_files(root_dir)
entries = dir(root_dir);
files = {};

for i = 1:numel(entries)
    name = entries(i).name;
    if entries(i).isdir
        if strcmp(name, '.') || strcmp(name, '..')
            continue;
        end
        sub_files = list_h5_files(fullfile(root_dir, name));
        if ~isempty(sub_files)
            files = [files, sub_files]; %#ok<AGROW>
        end
    else
        [~, ~, ext] = fileparts(name);
        if strcmpi(ext, '.h5')
            files{end + 1} = fullfile(root_dir, name); %#ok<AGROW>
        end
    end
end

end

function cleanup_temp_file(temp_file)
if exist(temp_file, 'file')
    delete(temp_file);
end
end

function [ok, message] = compare_binsparse_structs(a, b)
fields = {'values', 'indices_0', 'indices_1', 'pointers_to_1', ...
    'nrows', 'ncols', 'nnz', 'is_iso', 'format', 'structure'};

for i = 1:numel(fields)
    field = fields{i};
    if ~isfield(a, field)
        ok = false;
        message = ['missing field in first matrix: ', field];
        return;
    end
    if ~isfield(b, field)
        ok = false;
        message = ['missing field in second matrix: ', field];
        return;
    end
    if ~compare_field(a.(field), b.(field))
        ok = false;
        message = ['field mismatch: ', field];
        return;
    end
end

extra_a = setdiff(fieldnames(a), fields);
extra_b = setdiff(fieldnames(b), fields);
if ~isempty(extra_a) || ~isempty(extra_b)
    ok = false;
    message = 'field set mismatch';
    return;
end

ok = true;
message = '';
end

function ok = compare_field(a, b)
if ischar(a) || isstring(a) || ischar(b) || isstring(b)
    ok = strcmp(char(a), char(b));
else
    ok = isequaln(a, b);
end
end
