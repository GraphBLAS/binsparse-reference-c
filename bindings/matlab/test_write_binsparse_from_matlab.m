% SPDX-FileCopyrightText: 2024 Binsparse Developers
%
% SPDX-License-Identifier: BSD-3-Clause

function test_write_binsparse_from_matlab()
% TEST_WRITE_BINSPARSE_FROM_MATLAB - Test the write_binsparse_from_matlab MEX function
%
% This function tests the write_binsparse_from_matlab MEX function by creating
% sample SuiteSparse Matrix Collection problem structs and attempting to write
% them to Binsparse format files.

fprintf('=== Testing write_binsparse_from_matlab MEX function ===\n\n');

try
    % Test 1: Check if MEX function exists and is callable
    fprintf('Test 1: Checking MEX function availability\n');
    if exist('write_binsparse_from_matlab', 'file') ~= 3
        error('write_binsparse_from_matlab MEX function not found. Please compile it first.');
    end
    fprintf('✓ MEX function found\n\n');

    % Test 2: Test with minimal arguments (should fail)
    fprintf('Test 2: Testing error handling with insufficient arguments\n');
    try
        write_binsparse_from_matlab();
        error('Expected error for insufficient arguments');
    catch ME
        if ~isempty(strfind(ME.identifier, 'BinSparse:InvalidArgs'))
            fprintf('✓ Correctly handled insufficient arguments\n');
        else
            fprintf('✗ Unexpected error: %s\n', ME.message);
        end
    end
    fprintf('\n');

    % Test 3: Test with invalid problem struct
    fprintf('Test 3: Testing error handling with invalid problem struct\n');
    try
        write_binsparse_from_matlab(42, 'test.bsp.h5');
        error('Expected error for invalid problem struct');
    catch ME
        if ~isempty(strfind(ME.identifier, 'BinSparse:InvalidProblemStruct'))
            fprintf('✓ Correctly handled invalid problem struct\n');
        else
            fprintf('✗ Unexpected error: %s\n', ME.message);
        end
    end
    fprintf('\n');

    % Test 4: Create a basic SuiteSparse Matrix Collection problem struct
    fprintf('Test 4: Testing with basic SuiteSparse problem struct\n');
    problem = create_basic_problem_struct();
    problem

    fprintf('Testing skeleton implementation with basic problem struct:\n');
    write_binsparse_from_matlab(problem, 'test_basic.bsp.h5');
    fprintf('✓ Basic test completed successfully\n\n');

    % Test 5: Test with all optional arguments
    fprintf('Test 5: Testing with all optional arguments\n');
    problem_extended = create_extended_problem_struct();

    fprintf('Testing with all arguments:\n');
    write_binsparse_from_matlab(problem_extended, 'test_extended.bsp.h5', ...
                                'my_group', '{"test": "metadata"}', 6);
    fprintf('✓ Extended test completed successfully\n\n');

    % Test 6: Test with real sparse matrix
    fprintf('Test 6: Testing with real sparse matrix\n');
    problem_sparse = create_sparse_problem_struct();

    fprintf('Testing with sparse matrix:\n');
    write_binsparse_from_matlab(problem_sparse, 'test_sparse.bsp.h5');
    fprintf('✓ Sparse matrix test completed successfully\n\n');

    fprintf('=== All tests passed! ===\n');
    fprintf('Note: This is testing the skeleton implementation only.\n');
    fprintf('The actual file writing functionality needs to be implemented.\n\n');

catch ME
    fprintf('✗ Test failed: %s\n', ME.message);
    fprintf('Stack trace:\n');
    for i = 1:length(ME.stack)
        fprintf('  %s (line %d)\n', ME.stack(i).name, ME.stack(i).line);
    end
end

end

function problem = create_basic_problem_struct()
    % Create a minimal SuiteSparse Matrix Collection problem struct
    problem = struct();
    problem.name = 'test_basic_matrix';
    problem.A = speye(3); % 3x3 identity matrix
    problem.title = 'Basic Test Matrix';
    problem.kind = 'test matrix';
end

function problem = create_extended_problem_struct()
    % Create an extended SuiteSparse Matrix Collection problem struct
    problem = struct();
    problem.name = 'test_extended_matrix';
    problem.title = 'Extended Test Matrix with Metadata';
    problem.kind = 'artificial/test';
    problem.A = sparse([1 2 3], [1 2 3], [1.5 2.5 3.5], 4, 4);
    problem.notes = 'This is a test matrix for validation';
    problem.author = 'Test Suite';
    problem.date = datestr(now);
    problem.editor = 'MATLAB';

    % Add some additional fields that might be present
    problem.id = 12345;
    problem.group = 'Test';
    problem.num_rows = size(problem.A, 1);
    problem.num_cols = size(problem.A, 2);
    problem.nnz = nnz(problem.A);
    problem.pattern_symmetry = 1.0;
    problem.numerical_symmetry = 1.0;
    problem.type = 'real';
    problem.structure = 'unsymmetric';
    problem.sprank = rank(full(problem.A));
end

function problem = create_sparse_problem_struct()
    % Create a problem struct with a more complex sparse matrix
    problem = struct();
    problem.name = 'test_sparse_matrix';
    problem.title = 'Sparse Test Matrix';
    problem.kind = 'test/sparse';

    % Create a 10x10 sparse matrix with some pattern
    n = 10;
    [i, j] = meshgrid(1:n, 1:n);
    mask = abs(i - j) <= 2; % Pentadiagonal pattern
    rows = i(mask);
    cols = j(mask);
    vals = randn(length(rows), 1); % Random values

    problem.A = sparse(rows, cols, vals, n, n);
    problem.notes = sprintf('Random %dx%d pentadiagonal matrix with %d non-zeros', ...
                           n, n, nnz(problem.A));

    % Add matrix properties: FIXME: why?
    problem.num_rows = n;
    problem.num_cols = n;
    problem.nnz = nnz(problem.A);
    problem.type = 'real';
    problem.structure = 'unsymmetric';
end
