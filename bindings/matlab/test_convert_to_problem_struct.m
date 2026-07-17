function test_convert_to_problem_struct
%TEST_CONVERT_TO_PROBLEM_STRUCT test in-memory Binsparse Problem conversion

% SPDX-FileCopyrightText: 2026 Binsparse Developers
%
% SPDX-License-Identifier: BSD-3-Clause

metadata = struct('name', 'Test/converter', 'title', 'converter test', ...
    'id', 7, 'date', '2026', 'author', 'Binsparse Developers', ...
    'ed', 'Binsparse Developers', 'kind', 'test', ...
    'notes', {{'line one'; 'line two'}});

rows = [0; 0; 1; 2; 2];
cols = [0; 2; 1; 0; 2];
values = [1; 0; 2; 3; 4];
expected = sparse([1 2 3 3], [1 2 1 3], [1 2 3 4], 3, 3);
expected_zeros = sparse(1, 3, 1, 3, 3);

formats = matrix_formats(rows, cols, values, 3, 3);
for k = 1:numel(formats)
    raw = struct('metadata', metadata, 'A', formats{k});
    Problem = convert_to_problem_struct(raw);
    assert(isequal(Problem.A, expected), ...
        'A mismatch for %s', formats{k}.format);
    assert(isequal(Problem.Zeros, expected_zeros), ...
        'Zeros mismatch for %s', formats{k}.format);
    assert(isequal(Problem.notes, char({'line one'; 'line two'})));
end

raw = struct('metadata', metadata, ...
    'A', make_matrix(0, rows, cols, [], 3, 3, 'COO', true, 'general'));
Problem = convert_to_problem_struct(raw);
assert(nnz(Problem.A) == 0);
assert(isequal(Problem.Zeros, sparse(rows + 1, cols + 1, 1, 3, 3)));

lower = make_matrix([1; 2; 0; 3], [0; 1; 1; 2], [0; 0; 1; 2], ...
    [], 3, 3, 'COO', false, 'symmetric_lower');
raw = struct('metadata', metadata, 'A', lower);
Problem = convert_to_problem_struct(raw);
assert(isequal(Problem.A, sparse([1 2 1 3], [1 1 2 3], [1 2 2 3], 3, 3)));
assert(isequal(Problem.Zeros, sparse(2, 2, 1, 3, 3)));

hermitian = make_matrix([1; 2+3i; 4], [0; 1; 1], [0; 0; 1], ...
    [], 2, 2, 'COO', false, 'hermitian_lower');
raw = struct('metadata', metadata, 'A', hermitian);
Problem = convert_to_problem_struct(raw);
assert(isequal(Problem.A, sparse([1 2-3i; 2+3i 4])));

skew = make_matrix([5; 0], [1; 1], [0; 1], ...
    [], 2, 2, 'COO', false, 'skew_symmetric_lower');
raw = struct('metadata', metadata, 'A', skew);
Problem = convert_to_problem_struct(raw);
assert(isequal(Problem.A, sparse([0 -5; 5 0])));
assert(isequal(Problem.Zeros, sparse(2, 2, 1, 2, 2)));

cvec = make_matrix([8; 0], [0; 2], [], [], ...
    3, 1, 'CVEC', false, 'general');
raw = struct('metadata', metadata, 'A', cvec);
Problem = convert_to_problem_struct(raw);
assert(isequal(Problem.A, sparse(1, 1, 8, 3, 1)));
assert(isequal(Problem.Zeros, sparse(3, 1, 1, 3, 1)));

raw = struct('metadata', metadata, ...
    'A', formats{1}, ...
    'b', dense_matrix([10; 20; 30], 3, 1, 'DVEC'), ...
    'x', dense_matrix([1; 3; 2; 4], 2, 2, 'DMATC'));
raw.aux.seq_1 = dense_matrix([5; 6], 2, 1, 'DVEC');
raw.aux.seq_2 = dense_matrix([7; 8], 2, 1, 'DVEC');
raw.aux.label = {'abc'; 'def'};
Problem = convert_to_problem_struct(raw);
assert(isequal(Problem.b, [10; 20; 30]));
assert(isequal(Problem.x, [1 2; 3 4]));
assert(iscell(Problem.aux.seq) && numel(Problem.aux.seq) == 2);
assert(isequal(Problem.aux.seq{2}, [7; 8]));
assert(isequal(Problem.aux.label, char({'abc'; 'def'})));

dmat = dense_matrix([1; 2; 3; 4; 5; 6], 2, 3, 'DMAT');
raw = struct('metadata', metadata, 'A', formats{1}, 'b', dmat);
Problem = convert_to_problem_struct(raw);
assert(isequal(Problem.b, [1 2 3; 4 5 6]));

bad = formats{1};
bad.indices_1([1 2]) = bad.indices_1([2 1]);
assert_throws(@() convert_to_problem_struct( ...
    struct('metadata', metadata, 'A', bad)), 'BinSparse:InvalidMatrix');

bad = formats{3};
bad.pointers_to_1 = uint64([0; 3; 2; 5]);
assert_throws(@() convert_to_problem_struct( ...
    struct('metadata', metadata, 'A', bad)), 'BinSparse:InvalidMatrix');

bad = formats{5};
bad.pointers_to_1 = uint64([0; 2; 2; 5]);
assert_throws(@() convert_to_problem_struct( ...
    struct('metadata', metadata, 'A', bad)), 'BinSparse:InvalidMatrix');

bad = formats{1};
bad.format = 'CUSTOM';
assert_throws(@() convert_to_problem_struct( ...
    struct('metadata', metadata, 'A', bad)), 'BinSparse:UnsupportedFormat');

bad = dense_matrix((1:6).', 2, 3, 'DMATC');
assert_throws(@() convert_to_problem_struct( ...
    struct('metadata', metadata, 'A', bad)), 'BinSparse:InvalidProblem');

bad = make_matrix(uint64(9007199254740992) + uint64(1), 0, 0, [], ...
    1, 1, 'COO', false, 'general');
assert_throws(@() convert_to_problem_struct( ...
    struct('metadata', metadata, 'A', bad)), 'BinSparse:InexactValue');

fprintf('test_convert_to_problem_struct: all tests passed\n');

end

function assert_throws(f, identifier)
try
    f();
catch me
    assert(strcmp(me.identifier, identifier), ...
        'Expected %s, got %s', identifier, me.identifier);
    return;
end
error('Expected error %s was not thrown', identifier);
end

function formats = matrix_formats(rows, cols, values, m, n)
formats = cell(7, 1);
formats{1} = make_matrix(values, rows, cols, [], m, n, 'COO', false, 'general');
formats{2} = make_matrix(values([1 4 3 2 5]), cols([1 4 3 2 5]), ...
    rows([1 4 3 2 5]), [], m, n, 'COOC', false, 'general');
formats{3} = make_matrix(values, [], cols, [0; 2; 3; 5], ...
    m, n, 'CSR', false, 'general');
formats{4} = make_matrix(values([1 4 3 2 5]), [], rows([1 4 3 2 5]), ...
    [0; 2; 3; 5], m, n, 'CSC', false, 'general');
formats{5} = make_matrix(values, [0; 1; 2], cols, [0; 2; 3; 5], ...
    m, n, 'DCSR', false, 'general');
formats{6} = make_matrix(values([1 4 3 2 5]), [0; 1; 2], ...
    rows([1 4 3 2 5]), [0; 2; 3; 5], m, n, 'DCSC', false, 'general');
formats{7} = make_matrix(values, rows, cols, [], m, n, 'COOR', false, 'general');
end

function matrix = dense_matrix(values, m, n, format)
matrix = make_matrix(values, [], [], [], m, n, format, false, 'general');
end

function matrix = make_matrix(values, indices_0, indices_1, pointers, ...
                              m, n, format, is_iso, structure)
matrix = struct('values', values(:), ...
    'indices_0', uint64(indices_0(:)), ...
    'indices_1', uint64(indices_1(:)), ...
    'pointers_to_1', uint64(pointers(:)), ...
    'nrows', m, 'ncols', n, ...
    'nnz', max([numel(indices_0), numel(indices_1), numel(values)]), ...
    'is_iso', logical(is_iso), 'format', format, 'structure', structure);
if is_iso
    matrix.nnz = numel(indices_0);
end
if any(strcmp(format, {'DVEC', 'DMAT', 'DMATC'}))
    matrix.nnz = m * n;
end
end
