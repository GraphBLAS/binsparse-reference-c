% SPDX - FileCopyrightText : 2024 Binsparse Developers % % SPDX - License -
    Identifier
    : BSD -
      3 -
      Clause

              function
              test_bsp_hello() %
          TEST_BSP_HELLO
      -
      Test the bsp_hello MEX function % %
          This function tests the basic
          functionality of the bsp_hello MEX function
          % to verify that the Binsparse MATLAB bindings are working correctly.%
          % Usage : %
                    test_bsp_hello()

                        fprintf(
                            '=== Testing Binsparse MATLAB Bindings ===\n\n');

% Check if the MEX function exists if ~exist('bsp_hello', 'file') error(
      'bsp_hello MEX function not found. Run build_matlab_bindings() first.');
end

    fprintf('Testing bsp_hello MEX function...\n\n');

try % Test 1
    : Basic
          call with no arguments fprintf('Test 1: Basic call - bsp_hello()\n');
result1 = bsp_hello();
fprintf('  Result: %s\n', result1);
fprintf('  Status: PASS\n\n');

% Test 2
    : Version
          query fprintf('Test 2: Version query - bsp_hello(' 'version' ')\n');
[ version, success ] = bsp_hello('version');
fprintf('  Version: %s\n', version);
fprintf('  Success: %s\n', mat2str(success));
if success
  fprintf('  Status: PASS\n\n');
else
  fprintf('  Status: FAIL - Function reported error\n\n');
end

        % Test 3 : Error handling -
    invalid mode fprintf('Test 3: Error handling - bsp_hello(' 'invalid' ')\n');
try result3 = bsp_hello('invalid');
fprintf('  Status: FAIL - Should have thrown an error\n\n');
catch ME fprintf('  Caught expected error: %s\n', ME.message);
fprintf('  Status: PASS\n\n');
end

        % Test 4 : Type checking -
    numeric input fprintf('Test 4: Type checking - bsp_hello(42)\n');
try result4 = bsp_hello(42);
fprintf('  Status: FAIL - Should have thrown an error\n\n');
catch ME fprintf('  Caught expected error: %s\n', ME.message);
fprintf('  Status: PASS\n\n');
end

    fprintf('=== All Tests Completed ===\n');
fprintf('The Binsparse MATLAB bindings appear to be working correctly!\n\n');

% Display system information fprintf('System Information:\n');
fprintf('  MATLAB Version: %s\n', version('-release'));
fprintf('  MEX Extension: %s\n', mexext());
fprintf('  Platform: %s\n', computer());

catch ME fprintf('=== TEST FAILED ===\n');
fprintf('Error: %s\n', ME.message);
fprintf('Stack trace:\n');
    for
      i = 1 : length(ME.stack) fprintf('  %s (line %d)\n', ME.stack(i).name,
                                       ME.stack(i).line);
    end rethrow(ME);
    end

        end
