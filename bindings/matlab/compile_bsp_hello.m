% SPDX-FileCopyrightText: 2024 Binsparse Developers
%
% SPDX-License-Identifier: BSD-3-Clause

function compile_bsp_hello()
% COMPILE_BSP_HELLO - Compile the bsp_hello MEX function
%
% This script compiles the bsp_hello.c MEX function with proper
% include paths for the Binsparse library headers.
%
% Prerequisites:
% - MATLAB with MEX compiler configured (run 'mex -setup' if needed)
% - Binsparse C library headers available
%
% Usage:
%   compile_bsp_hello()

fprintf('Compiling bsp_hello MEX function...\n');

% Get the current directory(should be bindings / matlab) matlab_dir = pwd;
fprintf('Matlab bindings directory: %s\n', matlab_dir);

% Find the Binsparse include directory %
    Assuming
        we're in bindings/matlab, go up two levels to find include/ binsparse_root = fullfile(
            matlab_dir, '..', '..');
include_dir = fullfile(binsparse_root, 'include');

if
  ~exist(include_dir, 'dir')
      error('Binsparse include directory not found: %s', include_dir);
end

    fprintf('Using Binsparse include directory: %s\n', include_dir);

% MEX compilation command mex_cmd =
    sprintf('mex -I"%s" bsp_hello.c', include_dir);

fprintf('Running: %s\n', mex_cmd);

try % Compile the MEX function eval(mex_cmd);
fprintf('Successfully compiled bsp_hello MEX function!\n');

% Test if the function works fprintf('\nTesting the compiled function:\n');
result = bsp_hello();
fprintf('bsp_hello() returned: %s\n', result);

[ version, success ] = bsp_hello('version');
fprintf('bsp_hello(' 'version' ') returned: %s (success: %d)\n', version,
        success);

catch ME fprintf('Error during compilation:\n');
fprintf('%s\n', ME.message);
rethrow(ME);
end

    end
