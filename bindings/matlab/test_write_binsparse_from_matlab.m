function test_write_binsparse_from_matlab()
% TEST_WRITE_BINSPARSE_FROM_MATLAB - End-to-end test for the SSMC writer MEX

% SPDX-FileCopyrightText: 2024 Binsparse Developers
%
% SPDX-License-Identifier: BSD-3-Clause

fprintf('=== Testing write_binsparse_from_matlab MEX function ===\n\n');

required = {'write_binsparse_from_matlab', 'binsparse_read'};
for i = 1:numel(required)
    if exist(required{i}, 'file') ~= 3
        error('%s MEX function not found. Please compile it first.', required{i});
    end
end

try
    write_binsparse_from_matlab();
    error('Expected insufficient-argument error');
catch ME
    assert(strcmp(ME.identifier, 'BinSparse:InvalidArgs'), ...
           'Unexpected identifier: %s', ME.identifier);
end

try
    write_binsparse_from_matlab(42, 'invalid.bsp.h5');
    error('Expected invalid-problem error');
catch ME
    assert(strcmp(ME.identifier, 'BinSparse:InvalidProblem'), ...
           'Unexpected identifier: %s', ME.identifier);
end

Problem = struct();
Problem.name = 'test_basic_matrix';
Problem.title = 'Basic Test Matrix';
Problem.kind = 'test matrix';
Problem.A = sparse([1 3], [1 2], [5 7], 3, 3);
Problem.Zeros = sparse(2, 3, 1, 3, 3);
Problem.b = [1; 2; 3];
Problem.aux = struct();
Problem.aux.D = [10 20; 30 40];

out_file = [tempname() '.bsp.h5'];
cleanup_file = onCleanup(@() delete_if_exists(out_file));

write_binsparse_from_matlab(struct('Problem', Problem), out_file, 'COO', [], 0);

primary = bsp_to_matlab(binsparse_read(out_file));
assert(isequal(double(primary), full(Problem.A)), 'Primary matrix mismatch');

b = bsp_to_matlab(binsparse_read(out_file, 'b'));
assert(isequal(double(b), Problem.b), 'b vector mismatch');

D = bsp_to_matlab(binsparse_read(out_file, 'D'));
assert(isequal(double(D), Problem.aux.D), 'aux.D mismatch');

legacy_out_file = [tempname() '.bsp.h5'];
cleanup_legacy = onCleanup(@() delete_if_exists(legacy_out_file));
write_binsparse_from_matlab(Problem, legacy_out_file, 'legacy_group', '{}', 0);
assert(exist(legacy_out_file, 'file') == 2, 'Legacy call did not create file');

fprintf('Test passed.\n');

end

function mat = bsp_to_matlab(bsp)
    fmt = upper(bsp.format);
    switch fmt
        case 'COO'
            rows = double(bsp.indices_0) + 1;
            cols = double(bsp.indices_1) + 1;
            vals = bsp.values;
            mat = full(sparse(rows, cols, vals, bsp.nrows, bsp.ncols));
        case 'CSC'
            colptr = double(bsp.pointers_to_1);
            rowind = double(bsp.indices_1);
            rows = [];
            cols = [];
            vals = [];
            for j = 1:bsp.ncols
                idx = (colptr(j) + 1):colptr(j + 1);
                rows = [rows; rowind(idx) + 1];
                cols = [cols; j * ones(numel(idx), 1)];
                vals = [vals; bsp.values(idx)];
            end
            mat = full(sparse(rows, cols, vals, bsp.nrows, bsp.ncols));
        case 'DMAT'
            mat = reshape(bsp.values, [bsp.ncols, bsp.nrows]).';
        case 'DMATC'
            mat = reshape(bsp.values, [bsp.nrows, bsp.ncols]);
        case 'DVEC'
            mat = reshape(bsp.values, [bsp.nrows, 1]);
        otherwise
            error('Unsupported format in test: %s', fmt);
    end
end

function delete_if_exists(filename)
if exist(filename, 'file')
    delete(filename);
end
end
