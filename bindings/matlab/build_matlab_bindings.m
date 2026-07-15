% SPDX-FileCopyrightText: 2024 Binsparse Developers
%
% SPDX-License-Identifier: BSD-3-Clause

function build_matlab_bindings(varargin)
% BUILD_MATLAB_BINDINGS - Build Binsparse MATLAB MEX functions
%
% This script provides a simple interface to build MATLAB bindings
% for the Binsparse library. It automatically detects include paths
% and sets up the compilation environment.
%
% Usage:
%   build_matlab_bindings()           % Build all available MEX functions
%   build_matlab_bindings('verbose')  % Build with verbose output
%   build_matlab_bindings('clean')    % Clean compiled MEX files
%
% Prerequisites:
% - MATLAB with working MEX compiler (run 'mex -setup' if needed)
% - Binsparse C library headers (in ../../include/)
%
% Note: This script currently builds a simple demonstration MEX function.
%       Additional Binsparse functionality can be added by creating more
%       MEX wrapper functions.

% Parse input arguments
verbose = any(strcmpi(varargin, 'verbose'));
clean_only = any(strcmpi(varargin, 'clean'));

fprintf('=== Binsparse MATLAB Bindings Build Script ===\n\n');

if clean_only
    fprintf('Cleaning compiled MEX files...\n');
    clean_mex_files();
    return;
end

% Check MEX compiler
if ~check_mex_compiler()
    error('MEX compiler not properly configured. Run "mex -setup" first.');
end

% Find and validate paths
paths = get_build_paths();
if verbose
    fprintf('Build paths:\n');
    fprintf('  MATLAB dir: %s\n', paths.matlab_dir);
    fprintf('  Include dir: %s\n', paths.include_dir);
    fprintf('  Root dir: %s\n', paths.binsparse_root);
    fprintf('\n');
end

% Compile MEX functions
compile_mex_functions(paths, verbose);

fprintf('\n=== Build Complete ===\n');
fprintf('Run the test functions to verify the installation:\n');
fprintf('  test_binsparse_read()\n');
fprintf('  test_binsparse_write()\n\n');

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

    if ~exist(paths.include_dir, 'dir')
        error('Binsparse include directory not found: %s\nEnsure you are running this script from the bindings/matlab directory.', paths.include_dir);
    end

    % Check for main header file
    main_header = fullfile(paths.include_dir, 'binsparse', 'binsparse.h');
    if ~exist(main_header, 'file')
        error('Main Binsparse header not found: %s', main_header);
    end
end

function compile_mex_functions(paths, verbose)
    % Compile all MEX functions

    % List of MEX functions to compile
    mex_files = {'binsparse_read.c', 'binsparse_write.c'};

    fprintf('Compiling MEX functions...\n');

    for i = 1:length(mex_files)
        mex_file = mex_files{i};
        if ~exist(mex_file, 'file')
            warning('MEX source file not found: %s', mex_file);
            continue;
        end

        fprintf('  Compiling %s... ', mex_file);

        % Prepare MEX command with library linking
        lib_dir = fullfile(paths.binsparse_root, 'build');
        lib_path = fullfile(lib_dir, 'libbinsparse.a');
        cjson_lib = fullfile(lib_dir, '_deps', 'cjson-build', 'libcjson.so');

        mex_args = {'-I', paths.include_dir, mex_file, lib_path, cjson_lib, '-lhdf5_serial'};
        if verbose
            mex_args = [mex_args, {'-v'}];
        end

        try
            mex(mex_args{:});
            fprintf('SUCCESS\n');
        catch ME
            fprintf('FAILED\n');
            fprintf('    Error: %s\n', ME.message);
        end
    end
end

function clean_mex_files()
    % Clean compiled MEX files

    % Get MEX file extension for current platform
    mex_ext = mexext();

    % Find and delete MEX files
    mex_files = dir(['*.' mex_ext]);

    if isempty(mex_files)
        fprintf('No MEX files found to clean.\n');
        return;
    end

    fprintf('Removing %d MEX file(s):\n', length(mex_files));
    for i = 1:length(mex_files)
        fprintf('  %s\n', mex_files(i).name);
        delete(mex_files(i).name);
    end
end
