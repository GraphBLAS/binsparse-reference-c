% SPDX-FileCopyrightText: 2024 Binsparse Developers
%
% SPDX-License-Identifier: BSD-3-Clause

function test_binsparse_minimize_roundtrip()
% TEST_BINSPARSE_MINIMIZE_ROUNDTRIP - Test SSMC conversion + type minimization

fprintf('=== Testing binsparse_from_ssmc + binsparse_minimize_types ===\n\n');

if exist('binsparse_from_ssmc', 'file') ~= 3
    error('binsparse_from_ssmc MEX function not found. Please compile it first.');
end

if exist('binsparse_minimize_types', 'file') ~= 3
    error('binsparse_minimize_types MEX function not found. Please compile it first.');
end

if exist('binsparse_write', 'file') ~= 3 || exist('binsparse_read', 'file') ~= 3
    error('binsparse_write/binsparse_read MEX functions not found. Please compile them first.');
end

% Build a small matrix with explicit zeros pattern
n = 5;
A = sparse([1 3 5], [1 2 5], [1 2 3], n, n);
Zeros = sparse([2 4], [2 3], [1 1], n, n);

mat = binsparse_from_ssmc(A, Zeros, 'CSC');
mat_min = binsparse_minimize_types(mat);

assert(strcmp(mat_min.format, 'CSC'));
assert(mat_min.nrows == n && mat_min.ncols == n);
assert(mat_min.nnz == mat.nnz);

% Values should drop to single for exact float32 representable data
if ~strcmp(class(mat_min.values), 'single')
    error('Expected values to minimize to single, got %s', class(mat_min.values));
end

% Index arrays should shrink to unsigned integer types
if ~strcmp(class(mat_min.indices_1), 'uint8')
    error('Expected indices_1 to minimize to uint8, got %s', class(mat_min.indices_1));
end

if ~strcmp(class(mat_min.pointers_to_1), 'uint8')
    error('Expected pointers_to_1 to minimize to uint8, got %s', class(mat_min.pointers_to_1));
end

% Roundtrip through binsparse_write/binsparse_read
out_file = [tempname() '.bsp.h5'];
cleanup_file = onCleanup(@() delete_if_exists(out_file));

binsparse_write(out_file, mat_min);
mat_read = binsparse_read(out_file);

assert(mat_read.nrows == mat_min.nrows && mat_read.ncols == mat_min.ncols);
assert(mat_read.nnz == mat_min.nnz);
assert(strcmp(mat_read.format, mat_min.format));

fprintf('Test passed.\n');

end

function delete_if_exists(filename)
if exist(filename, 'file')
    delete(filename);
end
end
