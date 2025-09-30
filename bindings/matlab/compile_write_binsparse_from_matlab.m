% SPDX-FileCopyrightText: 2024 Binsparse Developers
%
% SPDX-License-Identifier: BSD-3-Clause

function compile_write_binsparse_from_matlab(varargin)
% COMPILE_WRITE_BINSPARSE_FROM_MATLAB - Compile the write_binsparse_from_matlab MEX function
%
% This script compiles the write_binsparse_from_matlab MEX function for MATLAB.
% It automatically detects include paths and links against the Binsparse library.
%
% Usage:
%   compile_write_binsparse_from_matlab()           % Standard compilation
%   compile_write_binsparse_from_matlab('verbose')  % Verbose compilation output
%   compile_write_binsparse_from_matlab('debug')    % Debug build with symbols
%
% Prerequisites:
% - MATLAB with working MEX compiler (run 'mex -setup' if needed)
% - Binsparse C library headers (in ../../include/)
% - Compiled Binsparse library (in ../../build/)

% Parse input arguments
verbose = any(strcmpi(varargin, 'verbose'));
debug_build = any(strcmpi(varargin, 'debug'));

fprintf('=== Compiling write_binsparse_from_matlab MEX function ===\n\n');

% Check if source file exists
source_file = 'write_binsparse_from_matlab.c';
if ~exist(source_file, 'file')
    error('Source file not found: %s\nEnsure you are in the correct directory.', source_file);
end

% Check MEX compiler
if ~check_mex_compiler()
    error('MEX compiler not properly configured. Run "mex -setup" first.');
end

% Get build paths
paths = get_build_paths();
if verbose
    fprintf('Build paths:\n');
    fprintf('  MATLAB dir: %s\n', paths.matlab_dir);
    fprintf('  Include dir: %s\n', paths.include_dir);
    fprintf('  Library dir: %s\n', paths.lib_dir);
    fprintf('  Root dir: %s\n', paths.binsparse_root);
    fprintf('\n');
end

% Compile the MEX function
compile_mex_function(source_file, paths, verbose, debug_build);

fprintf('\n=== Compilation Complete ===\n');
fprintf('Test the function with:\n');
fprintf('  test_write_binsparse_from_matlab()\n\n');

end

function success = check_mex_compiler()
    % Check if MEX compiler is configured
    try
        % Try to get MEX configuration
        cc = mex.getCompilerConfigurations('C');
        success = ~isempty(cc);
        if success
            fprintf('MEX compiler found: %s\n', cc(1).Name);
        end
    catch
        success = false;
    end
end

function paths = get_build_paths()
    % Get and validate build paths
    paths.matlab_dir = pwd;
    paths.binsparse_root = fullfile(paths.matlab_dir, '..', '..');
    paths.include_dir = fullfile(paths.binsparse_root, 'include');
    paths.lib_dir = fullfile(paths.binsparse_root, 'build');

    if ~exist(paths.include_dir, 'dir')
        error('Binsparse include directory not found: %s\nEnsure you are running this script from the bindings/matlab directory.', paths.include_dir);
    end

    if ~exist(paths.lib_dir, 'dir')
        error('Binsparse build directory not found: %s\nEnsure the library has been built first.', paths.lib_dir);
    end

    % Check for main header file
    main_header = fullfile(paths.include_dir, 'binsparse', 'binsparse.h');
    if ~exist(main_header, 'file')
        error('Main Binsparse header not found: %s', main_header);
    end

    % Check for compiled library
    lib_path = fullfile(paths.lib_dir, 'libbinsparse.a');
    if ~exist(lib_path, 'file')
        error('Binsparse library not found: %s\nEnsure the library has been built first.', lib_path);
    end
end

function compile_mex_function(source_file, paths, verbose, debug_build)
    % Compile the MEX function with appropriate flags and libraries

    fprintf('Compiling %s... ', source_file);

    % Prepare MEX command with library linking
    lib_path = fullfile(paths.lib_dir, 'libbinsparse.a');
    cjson_lib = fullfile(paths.lib_dir, '_deps', 'cjson-build', 'libcjson.so');

    % Check if we're on macOS and adjust cjson library path
    if ismac
        % On macOS, cjson might have a different extension or location
        cjson_alternatives = {
            fullfile(paths.lib_dir, '_deps', 'cjson-build', 'libcjson.dylib'),
            fullfile(paths.lib_dir, '_deps', 'cjson-build', 'libcjson.a')
        };

        for i = 1:length(cjson_alternatives)
            if exist(cjson_alternatives{i}, 'file')
                cjson_lib = cjson_alternatives{i};
                break;
            end
        end
    end

    % Build MEX arguments
    mex_args = {'-I', paths.include_dir, source_file, lib_path};

    % Add cjson library if it exists
    if exist(cjson_lib, 'file')
        mex_args{end+1} = cjson_lib;
    else
        warning('cjson library not found at expected location: %s', cjson_lib);
        fprintf('    Continuing without cjson library...\n');
    end

    % Add HDF5 library
    mex_args{end+1} = '-lhdf5_serial';

    % Add optional flags
    if verbose
        mex_args{end+1} = '-v';
    end

    if debug_build
        mex_args = [mex_args, {'-g', '-DDEBUG'}];
        fprintf('(debug build) ');
    end

    if verbose
        fprintf('\n    MEX command: ');
        for i = 1:length(mex_args)
            fprintf('%s ', mex_args{i});
        end
        fprintf('\n');
    end

    try
        mex(mex_args{:});
        fprintf('SUCCESS\n');
    catch ME
        fprintf('FAILED\n');
        fprintf('    Error: %s\n', ME.message);

        % Provide troubleshooting suggestions
        fprintf('\nTroubleshooting suggestions:\n');
        fprintf('  1. Ensure the Binsparse library has been built: cd ../../build && make\n');
        fprintf('  2. Check that MEX compiler is configured: mex -setup\n');
        fprintf('  3. Verify HDF5 development libraries are installed\n');
        fprintf('  4. Try running with verbose flag for more details\n');

        rethrow(ME);
    end
end
