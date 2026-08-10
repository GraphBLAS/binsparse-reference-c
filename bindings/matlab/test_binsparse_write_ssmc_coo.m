function test_binsparse_write_ssmc_coo
%TEST_BINSPARSE_WRITE_SSMC_COO test the direct row-sorted COO writer

% SPDX-FileCopyrightText: 2026 Binsparse Developers
%
% SPDX-License-Identifier: BSD-3-Clause

required = {'binsparse_write_ssmc_coo', 'binsparse_read'};
for k = 1:numel(required)
    if (exist(required{k}, 'file') ~= 3)
        error('%s MEX function not found', required{k});
    end
end

filename = [tempname '.bsp.h5'];
cleanup = onCleanup(@() delete_if_exists(filename)); %#ok<NASGU>

A = sparse([3 1 2 1], [1 2 2 4], [5 6 7 8], 4, 4);
Zeros = sparse([4 2], [1 3], [1 1], 4, 4);
binsparse_write_ssmc_coo(filename, A, Zeros, '', ...
    '{"metadata":{"role":"A"}}', 0);
raw = binsparse_read(filename);
assert(strcmp(raw.format, 'COO'));
assert(~raw.is_iso && isa(raw.values, 'single'));
assert(isa(raw.indices_0, 'uint8') && isa(raw.indices_1, 'uint8'));
assert(issortedrows([double(raw.indices_0), double(raw.indices_1)]));
assert(isequal(raw_to_sparse(raw), A));
assert(nnz(raw.values == 0) == nnz(Zeros));
assert(~isempty(regexp(h5readatt(filename, '/', 'binsparse'), ...
    '"role"\s*:\s*"A"', 'once')));

% An ISO matrix stores one value even though all COO coordinates are retained.
delete_if_exists(filename);
iso = sparse([3 1 2], [1 2 4], [9 9 9], 4, 4);
binsparse_write_ssmc_coo(filename, iso, [], '', '{}', 9);
raw_iso = binsparse_read(filename);
assert(raw_iso.is_iso && numel(raw_iso.values) == 1);
assert(isequal(raw_to_sparse(raw_iso), iso));

% Dimensions, rather than the number of entries, select coordinate widths.
delete_if_exists(filename);
wide = sparse(70000, 70000, 1, 70000, 70000);
binsparse_write_ssmc_coo(filename, wide, [], '', '{}', 0);
raw_wide = binsparse_read(filename);
assert(isa(raw_wide.indices_0, 'uint32'));
assert(isa(raw_wide.indices_1, 'uint32'));

% Complex values retain exactness and use the complex fallback safely.
delete_if_exists(filename);
complex_A = sparse([2 1], [1 2], [1+2i pi+4i], 2, 2);
binsparse_write_ssmc_coo(filename, complex_A, [], '', '{}', 0);
raw_complex = binsparse_read(filename);
assert(~raw_complex.is_iso && isa(raw_complex.values, 'double'));
assert(isequal(raw_to_sparse(raw_complex), complex_A));

% A named group appends to an existing primary file.
group_A = sparse([1 2], [2 1], [3 4], 2, 2);
binsparse_write_ssmc_coo(filename, group_A, [], 'aux', '{}', 0);
assert(isequal(raw_to_sparse(binsparse_read(filename, 'aux')), group_A));

duplicate = sparse(1, 1, 1, 2, 2);
assert_throws(@() binsparse_write_ssmc_coo( ...
    filename, duplicate, duplicate, 'duplicate', '{}', 0), ...
    'BinSparse:DuplicateIndex');

fprintf('test_binsparse_write_ssmc_coo: all tests passed\n');
end

function A = raw_to_sparse(raw)
values = raw.values(:);
if (raw.is_iso)
    values = repmat(values, raw.nnz, 1);
end
A = sparse(double(raw.indices_0) + 1, double(raw.indices_1) + 1, ...
    double(values), raw.nrows, raw.ncols);
end

function assert_throws(action, identifier)
try
    action();
catch me
    assert(strcmp(me.identifier, identifier), ...
        'Expected %s, received %s', identifier, me.identifier);
    return
end
error('Expected error %s', identifier);
end

function delete_if_exists(filename)
if (exist(filename, 'file') == 2)
    delete(filename);
end
end
