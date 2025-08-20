% SPDX-FileCopyrightText: 2024 Binsparse Developers
%
% SPDX-License-Identifier: BSD-3-Clause

function test_binsparse_read()
% TEST_BINSPARSE_READ - Test the binsparse_read MEX function
%
% This function demonstrates how to use the binsparse_read MEX function
% to read Binsparse format matrices into MATLAB/Octave.

fprintf('=== Testing Binsparse Read Function ===\n\n');

% Check if the MEX function exists
if ~exist('binsparse_read', 'file')
    error('binsparse_read MEX function not found. Build it first.');
end

try
    % Test error handling with invalid inputs
    fprintf('Test 1: Error handling\n');

    try
        matrix = binsparse_read();  % No arguments
        fprintf('FAILED - Should have thrown error\n');
    catch ME
        fprintf('  Correctly caught error for no arguments\n');
    end

    try
        matrix = binsparse_read(123);  % Non-string argument
        fprintf('FAILED - Should have thrown error\n');
    catch ME
        fprintf('  Correctly caught error for invalid filename type\n');
    end

    try
        matrix = binsparse_read('nonexistent_file.h5');  % File doesn't exist
        fprintf('FAILED - Should have thrown error\n');
    catch ME
        fprintf('  Correctly caught error for nonexistent file\n');
    end

    fprintf('  Status: PASS\n\n');

    fprintf('=== Basic Error Handling Tests Passed ===\n');
    fprintf('To test actual file reading, you need a valid Binsparse file.\n\n');

    fprintf('Usage examples:\n');
    fprintf('  matrix = binsparse_read(''myfile.h5'');           %% Read from HDF5\n');
    fprintf('  matrix = binsparse_read(''myfile.h5'', ''group''); %% Read from specific group\n');
    fprintf('  matrix = binsparse_read(''myfile.mtx'');          %% Read Matrix Market file\n\n');

    fprintf('The returned matrix will have these fields:\n');
    fprintf('  matrix.values       - Values array\n');
    fprintf('  matrix.indices_0    - First dimension indices\n');
    fprintf('  matrix.indices_1    - Second dimension indices\n');
    fprintf('  matrix.pointers_to_1 - Pointers for compressed formats\n');
    fprintf('  matrix.nrows        - Number of rows\n');
    fprintf('  matrix.ncols        - Number of columns\n');
    fprintf('  matrix.nnz          - Number of non-zeros\n');
    fprintf('  matrix.is_iso       - ISO matrix flag\n');
    fprintf('  matrix.format       - Matrix format string\n');
    fprintf('  matrix.structure    - Matrix structure string\n');

catch ME
    fprintf('=== TEST FAILED ===\n');
    fprintf('Error: %s\n', ME.message);
    rethrow(ME);
end

end
