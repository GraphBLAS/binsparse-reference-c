% SPDX-FileCopyrightText: 2024 Binsparse Developers
%
% SPDX-License-Identifier: BSD-3-Clause

function compile_binsparse_write()
% COMPILE_BINSPARSE_WRITE - Quick compilation script for binsparse_write
%
% This script compiles just the binsparse_write MEX function with proper
% library linking.

fprintf('Compiling binsparse_write MEX function...\n');

% Get paths
matlab_dir = pwd;
binsparse_root = fullfile(matlab_dir, '..', '..');
include_dir = fullfile(binsparse_root, 'include');
lib_dir = fullfile(binsparse_root, 'build');

% Check for required files
lib_path = fullfile(lib_dir, 'libbinsparse.a');
cjson_lib = fullfile(lib_dir, '_deps', 'cjson-build', 'libcjson.so');

if ~exist(lib_path, 'file')
    error('libbinsparse.a not found at: %s\nBuild the library first with cmake.', lib_path);
end

if ~exist(cjson_lib, 'file')
    error('libcjson.so not found at: %s\nBuild the library first with cmake.', cjson_lib);
end

fprintf('Using libraries:\n');
fprintf('  libbinsparse.a: %s\n', lib_path);
fprintf('  libcjson.so: %s\n', cjson_lib);

try
    % Compile with linking
    mex('-I', include_dir, 'binsparse_write.c', lib_path, cjson_lib, '-lhdf5_serial', '-v');
    fprintf('Successfully compiled binsparse_write!\n');

    % Test if it loads
    fprintf('Testing MEX function...\n');
    if exist('binsparse_write', 'file')
        fprintf('binsparse_write MEX function is ready to use.\n');
    else
        warning('MEX function compiled but not found in path.');
    end

catch ME
    fprintf('Compilation failed: %s\n', ME.message);
    rethrow(ME);
end

end
