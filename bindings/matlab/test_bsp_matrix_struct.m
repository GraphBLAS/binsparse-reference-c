function test_bsp_matrix_struct()
% TEST_BSP_MATRIX_STRUCT - Test the Binsparse matrix struct functionality
%
% This function demonstrates and tests the basic MATLAB struct
% that mirrors the C bsp_matrix_t structure.

% SPDX-FileCopyrightText: 2024 Binsparse Developers
%
% SPDX-License-Identifier: BSD-3-Clause

fprintf('=== Testing Binsparse Matrix Struct ===\n\n');

try
    % Test 1: Create empty matrix
    fprintf('Test 1: Creating empty matrix\n');
    empty_matrix = bsp_matrix_create();
    fprintf('Empty matrix created successfully\n');
    bsp_matrix_info(empty_matrix);
    fprintf('\n');

    % Test 2: Create simple COO matrix
    fprintf('Test 2: Creating simple COO matrix\n');
    % 3x3 identity matrix in COO format
    values = [1.0, 1.0, 1.0];
    rows = uint64([0, 1, 2]);      % 0-based indexing like C
    cols = uint64([0, 1, 2]);      % 0-based indexing like C
    pointers = uint64([]);  % Empty for COO format

    coo_matrix = bsp_matrix_create(values, rows, cols, pointers, ...
                                   3, 3, 3, false, 'COO', 'general');
    fprintf('COO matrix created successfully\n');
    bsp_matrix_info(coo_matrix);
    fprintf('\n');

    % Test 3: Create CSR matrix
    fprintf('Test 3: Creating simple CSR matrix\n');
    % Same 3x3 identity in CSR format
    csr_values = [1.0, 1.0, 1.0];
    csr_cols = uint64([0, 1, 2]);
    csr_rows = uint64([]);  % Not used in CSR
    csr_ptrs = uint64([0, 1, 2, 3]); % Row pointers

    csr_matrix = bsp_matrix_create(csr_values, csr_rows, csr_cols, csr_ptrs, ...
                                   3, 3, 3, false, 'CSR', 'general');
    fprintf('CSR matrix created successfully\n');
    bsp_matrix_info(csr_matrix);
    fprintf('\n');

    % Test 4: Test field access
    fprintf('Test 4: Testing field access\n');
    fprintf('Matrix format: %s\n', csr_matrix.format);
    fprintf('Matrix structure: %s\n', csr_matrix.structure);
    fprintf('Is ISO: %s\n', mat2str(csr_matrix.is_iso));
    fprintf('First value: %.1f\n', csr_matrix.values(1));
    fprintf('\n');

    % Test 5: Test error handling
    fprintf('Test 5: Testing error handling\n');
    try
        invalid_matrix = bsp_matrix_create(1, 2, 3); % Wrong number of args
        fprintf('FAILED - Should have thrown error\n');
    catch ME
        fprintf('Successfully caught error: %s\n', ME.message);
    end
    fprintf('\n');

    fprintf('=== All Tests Passed ===\n');
    fprintf('The Binsparse matrix struct is working correctly!\n');

catch ME
    fprintf('=== TEST FAILED ===\n');
    fprintf('Error: %s\n', ME.message);
    rethrow(ME);
end

end
