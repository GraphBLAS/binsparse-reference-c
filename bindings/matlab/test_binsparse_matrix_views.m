function test_binsparse_matrix_views
%TEST_BINSPARSE_MATRIX_VIEWS exercise non-owning MATLAB BSP matrix views

% SPDX-FileCopyrightText: 2026 Binsparse Developers
%
% SPDX-License-Identifier: BSD-3-Clause

required = {'binsparse_minimize_types', 'binsparse_write', 'binsparse_read'};
for k = 1:numel(required)
    if (exist(required{k}, 'file') ~= 3)
        error('%s MEX function not found', required{k});
    end
end

filename = [tempname '.bsp.h5'];
cleanup = onCleanup(@() delete_if_exists(filename)); %#ok<NASGU>

% All four real arrays are borrowed.  Values and indices are replaced during
% minimization, while the uint8 pointers remain a view until the output struct
% is constructed.  Neither operation may alter or take ownership of the input.
input = make_matrix([1; 2; 4], uint64([]), uint64([0; 2; 1]), ...
    uint8([0; 1; 2; 3]), 3, 3, 'CSC');
output = binsparse_minimize_types(input);
assert(isa(input.values, 'double') && isequal(input.values, [1; 2; 4]));
assert(isa(input.indices_1, 'uint64') && ...
    isequal(input.indices_1, uint64([0; 2; 1])));
assert(isa(input.pointers_to_1, 'uint8') && ...
    isequal(input.pointers_to_1, uint8([0; 1; 2; 3])));
assert(isa(output.values, 'single'));
assert(isa(output.indices_1, 'uint8'));
assert(isa(output.pointers_to_1, 'uint8'));

binsparse_write(filename, output);
assert(isequal(output.values, single([1; 2; 4])));
assert(isequal(output.indices_1, uint8([0; 2; 1])));
roundtrip = binsparse_read(filename);
assert(isequal(roundtrip.values, output.values));
assert(isequal(roundtrip.indices_1, output.indices_1));

% If minimization changes nothing, every output data array is constructed from
% a view.  The output must remain independently owned after the input is cleared.
unchanged = make_matrix(single([1; 2; 4]), uint8([0; 1; 2]), ...
    uint8([0; 1; 2]), uint8([]), 3, 3, 'COO');
unchanged_output = binsparse_minimize_types(unchanged);
clear unchanged
assert(isequal(unchanged_output.values, single([1; 2; 4])));
assert(isequal(unchanged_output.indices_0, uint8([0; 1; 2])));
assert(isequal(unchanged_output.indices_1, uint8([0; 1; 2])));

% Complex MATLAB storage is deliberately copied rather than viewed.  Exercise
% both replacement by the minimizer and cleanup by the writer.
complex_input = make_matrix([1+2i; 3+4i], uint8([0; 1]), ...
    uint8([0; 1]), uint8([]), 2, 2, 'COO');
complex_output = binsparse_minimize_types(complex_input);
assert(isa(complex_input.values, 'double'));
assert(isequal(complex_input.values, [1+2i; 3+4i]));
assert(isa(complex_output.values, 'single'));
delete_if_exists(filename);
binsparse_write(filename, complex_output);
complex_roundtrip = binsparse_read(filename);
assert(isequal(complex_roundtrip.values, complex_output.values));

% Sparse arrays are invalid inside a raw BSP struct.  Rejecting them prevents a
% view from treating a sparse value buffer as a dense numel-sized allocation.
invalid = make_matrix(sparse(eye(2)), uint8([0; 1]), ...
    uint8([0; 1]), uint8([]), 2, 2, 'COO');
assert_throws(@() binsparse_minimize_types(invalid), ...
    'BinSparse:ConversionError');
assert_throws(@() binsparse_write(filename, invalid), ...
    'BinSparse:ConversionError');

fprintf('test_binsparse_matrix_views: all tests passed\n');
end

function matrix = make_matrix(values, indices_0, indices_1, pointers, ...
                              nrows, ncols, format)
matrix = struct('values', values, 'indices_0', indices_0, ...
    'indices_1', indices_1, 'pointers_to_1', pointers, ...
    'nrows', nrows, 'ncols', ncols, 'nnz', numel(values), ...
    'is_iso', false, 'format', format, 'structure', 'general');
end

function assert_throws(f, identifier)
try
    f();
catch me
    assert(strcmp(me.identifier, identifier), ...
        'Expected %s, got %s', identifier, me.identifier);
    return
end
error('Expected error %s', identifier);
end

function delete_if_exists(filename)
if (exist(filename, 'file') == 2)
    delete(filename);
end
end
