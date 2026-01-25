% SPDX-FileCopyrightText: 2024 Binsparse Developers
%
% SPDX-License-Identifier: BSD-3-Clause

function test_binsparse_from_ssmc()
% TEST_BINSPARSE_FROM_SSMC - Basic test for binsparse_from_ssmc MEX function

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

fprintf('Test passed.\n');

end
