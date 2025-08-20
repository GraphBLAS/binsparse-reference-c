% SPDX-FileCopyrightText: 2024 Binsparse Developers
%
% SPDX-License-Identifier: BSD-3-Clause

function test_bsp_hello_octave()
% TEST_BSP_HELLO_OCTAVE - Test the bsp_hello MEX function in Octave
%
% This function tests the basic functionality of the bsp_hello MEX function
% to verify that the Binsparse Octave bindings are working correctly.
%
% Usage:
%   test_bsp_hello_octave()

fprintf('=== Testing Binsparse Octave Bindings ===\n\n');

% Check if we're running in Octave
if ~(exist('OCTAVE_VERSION', 'builtin') ~= 0)
    warning('This test is designed for Octave. For MATLAB, use test_bsp_hello.m');
end

% Check if the MEX function exists
if ~exist('bsp_hello', 'file')
    error('bsp_hello MEX function not found. Run build_octave_bindings() first.');
end

fprintf('Testing bsp_hello MEX function in Octave...\n\n');

try
    % Test 1: Basic call with no arguments
    fprintf('Test 1: Basic call - bsp_hello()\n');
    result1 = bsp_hello();
    fprintf('  Result: %s\n', result1);
    fprintf('  Status: PASS\n\n');
    
    % Test 2: Version query
    fprintf('Test 2: Version query - bsp_hello(''version'')\n');
    [version, success] = bsp_hello('version');
    fprintf('  Version: %s\n', version);
    fprintf('  Success: %s\n', mat2str(success));
    if success
        fprintf('  Status: PASS\n\n');
    else
        fprintf('  Status: FAIL - Function reported error\n\n');
    end
    
    % Test 3: Error handling - invalid mode
    fprintf('Test 3: Error handling - bsp_hello(''invalid'')\n');
    try
        result3 = bsp_hello('invalid');
        fprintf('  Status: FAIL - Should have thrown an error\n\n');
    catch
        lasterr_msg = lasterr();
        fprintf('  Caught expected error: %s\n', lasterr_msg);
        fprintf('  Status: PASS\n\n');
    end
    
    % Test 4: Type checking - numeric input
    fprintf('Test 4: Type checking - bsp_hello(42)\n');
    try
        result4 = bsp_hello(42);
        fprintf('  Status: FAIL - Should have thrown an error\n\n');
    catch
        lasterr_msg = lasterr();
        fprintf('  Caught expected error: %s\n', lasterr_msg);
        fprintf('  Status: PASS\n\n');
    end
    
    fprintf('=== All Tests Completed ===\n');
    fprintf('The Binsparse Octave bindings appear to be working correctly!\n\n');
    
    % Display system information
    fprintf('System Information:\n');
    if exist('OCTAVE_VERSION', 'builtin')
        fprintf('  Octave Version: %s\n', OCTAVE_VERSION);
    end
    fprintf('  Platform: %s\n', computer());
    
    % Check for mkoctfile
    [status, output] = system('mkoctfile --version 2>/dev/null || mkoctfile --version 2>nul');
    if status == 0
        % Extract version from output (first line usually)
        lines = strsplit(output, '\n');
        if ~isempty(lines)
            fprintf('  mkoctfile: %s\n', strtrim(lines{1}));
        end
    end
    
catch
    lasterr_msg = lasterr();
    fprintf('=== TEST FAILED ===\n');
    fprintf('Error: %s\n', lasterr_msg);
    
    % In Octave, we don't have ME.stack, so provide simpler error info
    fprintf('Last error occurred in test_bsp_hello_octave\n');
    rethrow(lasterr());
end

end