% SPDX-FileCopyrightText: 2024 Binsparse Developers
%
% SPDX-License-Identifier: BSD-3-Clause

function compile_binsparse_write_octave()
% COMPILE_BINSPARSE_WRITE_OCTAVE - Quick Octave compilation for binsparse_write
%
% This script compiles just the binsparse_write MEX function using mkoctfile
% with proper library linking for Octave.

fprintf('Compiling binsparse_write MEX function for Octave...\n');

% Check if we're in Octave
if ~(exist('OCTAVE_VERSION', 'builtin') ~= 0)
    warning('This script is designed for Octave. For MATLAB, use compile_binsparse_write.m');
end

% Get paths
matlab_dir = pwd;
binsparse_root = fullfile(matlab_dir, '..', '..');
include_dir = fullfile(binsparse_root, 'include');
lib_dir = fullfile(binsparse_root, 'build');

% Check for required files
lib_path = fullfile(lib_dir, 'libbinsparse.a');
cjson_lib_path = fullfile(lib_dir, '_deps', 'cjson-build', 'libcjson.a');

if ~exist(lib_path, 'file')
    error('libbinsparse.a not found at: %s\nBuild the library first with cmake.', lib_path);
end

if ~exist(cjson_lib_path, 'file')
    error('libcjson.a not found at: %s\nRebuild with static cJSON support.', cjson_lib_path);
end

fprintf('Using libraries:\n');
fprintf('  libbinsparse.a: %s\n', lib_path);
fprintf('  libcjson.a: %s\n', cjson_lib_path);

% Build command for mkoctfile with -fPIC and proper static linking
cmd = sprintf('mkoctfile --mex -fPIC -I%s binsparse_write.c -Wl,--whole-archive %s %s -Wl,--no-whole-archive -lhdf5_serial', ...
              include_dir, lib_path, cjson_lib_path);

fprintf('Running: %s\n', cmd);

try
    % Execute mkoctfile
    [status, output] = system(cmd);

    if status == 0
        fprintf('Successfully compiled binsparse_write!\n');
        if ~isempty(output)
            fprintf('Output: %s\n', output);
        end

        % Test if it loads
        fprintf('Testing MEX function...\n');
        if exist('binsparse_write', 'file')
            fprintf('binsparse_write MEX function is ready to use.\n');
        else
            warning('MEX function compiled but not found in path.');
        end
    else
        fprintf('Compilation failed with status %d\n', status);
        if ~isempty(output)
            fprintf('Error output: %s\n', output);
        end
        error('mkoctfile compilation failed');
    end

catch ME
    fprintf('Compilation failed: %s\n', ME.message);
    rethrow(ME);
end

end
