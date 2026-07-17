function test_generate_bsp_from_ssmc()
% TEST_GENERATE_BSP_FROM_SSMC - End-to-end test for generate_bsp_from_ssmc

% SPDX-FileCopyrightText: 2024 Binsparse Developers
%
% SPDX-License-Identifier: BSD-3-Clause

fprintf('=== Testing generate_bsp_from_ssmc ===\n\n');

required = {'binsparse_from_ssmc', 'binsparse_minimize_types', ...
            'binsparse_write', 'binsparse_read', ...
            'binsparse_write_string_dataset', 'generate_bsp_from_ssmc'};
for i = 1:numel(required)
    if exist(required{i}, 'file') ~= 3 && exist(required{i}, 'file') ~= 2
        error('%s not found. Please compile MEX functions and ensure the .m file is on path.', required{i});
    end
end

% Build synthetic problem
Problem = struct();
Problem.name = 'Test/Small';
Problem.title = 'Small test';
Problem.id = 7;
Problem.date = '2026-06-03';
Problem.author = 'Binsparse Developers';
Problem.ed = 'Binsparse Developers';
Problem.kind = 'test matrix';
Problem.notes = ['first note  '; 'second note '];
Problem.A = sparse([1 3 4], [1 2 4], [5 6 7], 4, 4);
Problem.Zeros = sparse([2 4], [2 1], [1 1], 4, 4);
Problem.b = [10; 20; 30; 40];
Problem.x = [1 2; 3 4; 5 6; 7 8];
Problem.aux = struct();
Problem.aux.c = [1; 2; 3];
Problem.aux.D = [1 0 2; 3 4 5];
Problem.aux.S = sparse([1 2], [2 3], [9 8], 3, 3);
Problem.aux.note = ['hello '; 'there '];
Problem.aux.tags = {'alpha'; 'beta'};

problem = struct('Problem', Problem);

out_file = [tempname() '.bsp.h5'];
cleanup_file = onCleanup(@() delete_if_exists(out_file));

format = 'COO';
compression_level = 0;

generate_bsp_from_ssmc(problem, out_file, format, compression_level);

% Read primary
primary_bsp = binsparse_read(out_file);
primary_mat = bsp_to_matlab(primary_bsp);
expected_primary = full(Problem.A);
assert(matrices_equal(primary_mat, expected_primary), 'Primary matrix mismatch');

% Read aux and x/b
check_dense_group(out_file, 'b', Problem.b(:));
check_dense_group(out_file, 'x', Problem.x);
check_dense_group(out_file, 'c', Problem.aux.c(:));
check_dense_group(out_file, 'D', Problem.aux.D);

aux_sparse = binsparse_read(out_file, 'S');
aux_sparse_mat = bsp_to_matlab(aux_sparse);
expected_sparse = full(Problem.aux.S);
assert(matrices_equal(aux_sparse_mat, expected_sparse), 'Aux sparse matrix mismatch');

check_string_dataset(out_file, 'note', {'hello '; 'there '});
check_string_dataset(out_file, 'tags', {'alpha'; 'beta'});

json = h5readatt(out_file, '/', 'binsparse');
assert(contains(json, '"metadata"'), 'Primary metadata not nested');
assert(~isempty(regexp(json, '"id"\s*:\s*7', 'once')), ...
       'Primary metadata id is not a JSON number');
assert(~contains(json, '"ssmc_metadata"'), 'Unexpected legacy metadata key');

fprintf('Test passed.\n');

end

function check_dense_group(filename, group, expected)
    bsp = binsparse_read(filename, group);
    actual = bsp_to_matlab(bsp);
    assert(matrices_equal(actual, expected), ...
           'Group "%s" mismatch', group);
end

function check_string_dataset(filename, name, expected)
    actual = h5read(filename, ['/' name]);
    actual = as_cellstr(actual);
    assert(isequal(actual, expected), 'String dataset "%s" mismatch', name);
end

function value = as_cellstr(value)
    if iscell(value)
        value = value(:);
    elseif isstring(value)
        value = cellstr(value(:));
    elseif ischar(value)
        value = cellstr(value);
    else
        error('Unexpected string dataset value type');
    end
end

function ok = matrices_equal(a, b)
    if ~isequal(size(a), size(b))
        ok = false;
        return;
    end
    a = double(a);
    b = double(b);
    diff = a - b;
    ok = all(diff(:) == 0);
end

function mat = bsp_to_matlab(bsp)
    fmt = upper(bsp.format);
    switch fmt
        case 'COO'
            rows = double(bsp.indices_0) + 1;
            cols = double(bsp.indices_1) + 1;
            vals = bsp.values;
            mat = sparse(rows, cols, vals, bsp.nrows, bsp.ncols);
            mat = full(mat);
        case 'CSC'
            colptr = double(bsp.pointers_to_1);
            rowind = double(bsp.indices_1);
            vals = bsp.values;
            ncols = bsp.ncols;
            nrows = bsp.nrows;
            rows = [];
            cols = [];
            v = [];
            for j = 1:ncols
                start_idx = colptr(j) + 1;
                end_idx = colptr(j + 1);
                if end_idx >= start_idx
                    idx = start_idx:end_idx;
                    rows = [rows; rowind(idx) + 1];
                    cols = [cols; j * ones(numel(idx), 1)];
                    v = [v; vals(idx)];
                end
            end
            mat = sparse(rows, cols, v, nrows, ncols);
            mat = full(mat);
        case 'CSR'
            rowptr = double(bsp.pointers_to_1);
            colind = double(bsp.indices_1);
            vals = bsp.values;
            nrows = bsp.nrows;
            ncols = bsp.ncols;
            rows = [];
            cols = [];
            v = [];
            for i = 1:nrows
                start_idx = rowptr(i) + 1;
                end_idx = rowptr(i + 1);
                if end_idx >= start_idx
                    idx = start_idx:end_idx;
                    rows = [rows; i * ones(numel(idx), 1)];
                    cols = [cols; colind(idx) + 1];
                    v = [v; vals(idx)];
                end
            end
            mat = sparse(rows, cols, v, nrows, ncols);
            mat = full(mat);
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
