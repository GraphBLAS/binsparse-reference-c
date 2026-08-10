function test_binsparse_from_ssmc()
% TEST_BINSPARSE_FROM_SSMC - Basic test for binsparse_from_ssmc MEX function

% SPDX-FileCopyrightText: 2024 Binsparse Developers
%
% SPDX-License-Identifier: BSD-3-Clause

fprintf('=== Testing binsparse_from_ssmc MEX function ===\n\n');

if exist('binsparse_from_ssmc', 'file') ~= 3
    error('binsparse_from_ssmc MEX function not found. Please compile it first.');
end

n = 4;
A = sparse([1 3], [2 4], [10 20], n, n);
Zeros = sparse([2 4], [1 3], [1 1], n, n);

mat = binsparse_from_ssmc(A, Zeros, 'CSC');

assert(isstruct(mat));
assert(mat.nrows == n && mat.ncols == n);
assert(strcmp(mat.format, 'CSC'));
assert(mat.nnz == nnz(A) + nnz(Zeros));

% Validate that explicit zeros were inserted
zero_values = mat.values(mat.values == 0);
if numel(zero_values) ~= nnz(Zeros)
    error('Expected %d explicit zero values, got %d', nnz(Zeros), numel(zero_values));
end

assert(isa(mat.indices_1, 'uint8'));
assert(isa(mat.pointers_to_1, 'uint8'));

% ISO values are detected before full conversion arrays are allocated.
iso = binsparse_from_ssmc(sparse([1 3], [2 4], [7 7], n, n), 'COO');
assert(iso.is_iso && isequal(iso.values, 7));
assert(isa(iso.indices_0, 'uint8') && isa(iso.indices_1, 'uint8'));

% Explicit zeros preserve ISO only when every stored value is also zero.
zero_iso = binsparse_from_ssmc(sparse(n, n), Zeros, 'CSC');
assert(zero_iso.is_iso && isequal(zero_iso.values, 0));

fprintf('Test passed.\n');

end
