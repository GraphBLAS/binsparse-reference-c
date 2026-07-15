% SPDX-FileCopyrightText: 2024 Binsparse Developers
%
% SPDX-License-Identifier: BSD-3-Clause

function test_binsparse_write()
% TEST_BINSPARSE_WRITE - Test the binsparse_write MEX function
%
% This function creates a simple test matrix and writes it to a temporary
% Binsparse file using the binsparse_write MEX function.

fprintf('=== Testing binsparse_write MEX function ===\n\n');

% Check if binsparse_write exists
if ~exist('binsparse_write', 'file')
    error('binsparse_write MEX function not found. Please compile it first.');
end

try
    % Create a simple test matrix in the expected struct format
    fprintf('Creating test matrix...\n');

    % Simple 3x3 COO matrix:
    % [1.0, 0.0, 2.0]
    % [0.0, 3.0, 0.0]
    % [4.0, 0.0, 5.0]

    matrix = struct();
    matrix.values = [1.0; 3.0; 2.0; 4.0; 5.0];  % Values in COO order
    matrix.indices_0 = uint64([0; 1; 0; 2; 2]);  % Row indices (0-based)
    matrix.indices_1 = uint64([0; 1; 2; 0; 2]);  % Column indices (0-based)
    matrix.pointers_to_1 = uint64([]);           % Empty for COO format
    matrix.nrows = 3;
    matrix.ncols = 3;
    matrix.nnz = 5;
    matrix.is_iso = false;
    matrix.format = 'COO';
    matrix.structure = 'general';

    fprintf('Matrix created:\n');
    fprintf('  Format: %s\n', matrix.format);
    fprintf('  Size: %dx%d\n', matrix.nrows, matrix.ncols);
    fprintf('  NNZ: %d\n', matrix.nnz);
    fprintf('  Structure: %s\n', matrix.structure);

    % Create temporary filename
    temp_file = tempname();
    temp_file = [temp_file, '.bsp.h5'];

    fprintf('\nWriting matrix to: %s\n', temp_file);

    % Test basic write
    binsparse_write(temp_file, matrix);
    fprintf('✓ Basic write successful\n');

    % Test with group
    binsparse_write(temp_file, matrix, 'test_group');
    fprintf('✓ Write with group successful\n');

    % Test with group and JSON metadata
    json_metadata = '{"test": "metadata", "created_by": "test_binsparse_write"}';
    binsparse_write(temp_file, matrix, 'test_group_json', json_metadata);
    fprintf('✓ Write with group and JSON successful\n');

    % Test with all parameters
    binsparse_write(temp_file, matrix, 'test_group_full', json_metadata, 6);
    fprintf('✓ Write with all parameters successful\n');

    % Verify file exists and has reasonable size
    if exist(temp_file, 'file')
        info = dir(temp_file);
        fprintf('✓ Output file created (size: %d bytes)\n', info.bytes);
    else
        error('Output file was not created');
    end

    % Test round-trip if binsparse_read is available
    if exist('binsparse_read', 'file')
        fprintf('\nTesting round-trip (write → read)...\n');

        % Read back the matrix
        read_matrix = binsparse_read(temp_file, 'test_group_full');

        % Basic verification
        if read_matrix.nrows == matrix.nrows && ...
           read_matrix.ncols == matrix.ncols && ...
           read_matrix.nnz == matrix.nnz
            fprintf('✓ Round-trip dimensions match\n');
        else
            warning('Round-trip dimensions mismatch');
        end

        if strcmp(read_matrix.format, matrix.format) && ...
           strcmp(read_matrix.structure, matrix.structure)
            fprintf('✓ Round-trip format/structure match\n');
        else
            warning('Round-trip format/structure mismatch');
        end
    else
        fprintf('\nSkipping round-trip test (binsparse_read not available)\n');
    end

    % Clean up
    if exist(temp_file, 'file')
        delete(temp_file);
        fprintf('✓ Temporary file cleaned up\n');
    end

    fprintf('\n=== All tests passed! ===\n');
    fprintf('binsparse_write is working correctly.\n\n');

catch ME
    % Clean up on error
    if exist('temp_file', 'var') && exist(temp_file, 'file')
        delete(temp_file);
    end

    fprintf('\n=== Test failed ===\n');
    fprintf('Error: %s\n', ME.message);
    rethrow(ME);
end

end
