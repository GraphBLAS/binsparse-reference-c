function binsparse_build_matlab_bindings(varargin)
% BINSPARSE_BUILD_MATLAB_BINDINGS - Build Binsparse MATLAB MEX functions
%
% This script provides a simple interface to build MATLAB bindings
% for the Binsparse library. It automatically detects include paths
% and sets up the compilation environment.
%
% Usage:
%   binsparse_build_matlab_bindings()           % Build all available MEX functions
%   binsparse_build_matlab_bindings('verbose')  % Build with verbose output
%   binsparse_build_matlab_bindings('clean')    % Clean compiled MEX files
%
% Prerequisites:
% - MATLAB with working MEX compiler (run 'mex -setup' if needed)
% - Binsparse C library headers (in ../../include/)
% - Compiled Binsparse library (in ../../build/)

% SPDX-FileCopyrightText: 2024 Binsparse Developers
%
% SPDX-License-Identifier: BSD-3-Clause

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
    if ~isempty(paths.hdf5_include_dir)
        fprintf('  HDF5 include dir: %s\n', paths.hdf5_include_dir);
    end
    fprintf('  Root dir: %s\n', paths.binsparse_root);
    fprintf('\n');
end

% Compile MEX functions
compile_mex_functions(paths, verbose);

fprintf('\n=== Build Complete ===\n');
fprintf('Run the test functions to verify the installation:\n');
fprintf('  test_binsparse_read()\n');
fprintf('  test_binsparse_write()\n');
fprintf('  test_binsparse_write_ssmc_problem()\n\n');

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
    paths.hdf5_include_dir = hdf5_include_dir();

    if ~exist(paths.include_dir, 'dir')
        error('Binsparse include directory not found: %s\nEnsure you are running this script from the bindings/matlab directory.', paths.include_dir);
    end

    % Check for main header file
    main_header = fullfile(paths.include_dir, 'binsparse', 'binsparse.h');
    if ~exist(main_header, 'file')
        error('Main Binsparse header not found: %s', main_header);
    end
end

function include_dir = hdf5_include_dir()
    include_dir = '';
    if exist('/usr/include/hdf5/serial', 'dir')
        include_dir = '/usr/include/hdf5/serial';
    elseif exist('/usr/include/hdf5', 'dir')
        include_dir = '/usr/include/hdf5';
    end
end

function compile_mex_functions(paths, verbose)
    % Compile all MEX functions

    % List of MEX functions to compile
    mex_files = {'binsparse_read.c', 'binsparse_write.c', ...
        'binsparse_from_ssmc.c', 'binsparse_minimize_types.c', ...
        'binsparse_write_string_dataset.c'};

    fprintf('Compiling MEX functions...\n');
    failed_files = {};

    for i = 1:length(mex_files)
        mex_file = mex_files{i};
        if ~exist(mex_file, 'file')
            warning('MEX source file not found: %s', mex_file);
            continue;
        end

        fprintf('  Compiling %s... ', mex_file);

        % Prepare MEX command with library linking.  The MEX functions link
        % against the shared Binsparse library, so embed an rpath to the
        % build directory on platforms that support it.
        lib_dir = fullfile(paths.binsparse_root, 'build');
        lib_path = fullfile(lib_dir, 'libbinsparse_dynamic.so');
        cjson_lib = fullfile(lib_dir, '_deps', 'cjson-build', 'libcjson.a');
        if (ismac)
            rpath = '-rpath ' ;
        elseif (isunix)
            rpath = '-rpath=' ;
        else
            rpath = '' ;
        end
        if ~isempty(rpath)
            rpath = sprintf (' -Wl,%s''''%s'''' ', rpath, lib_dir) ;
            rpath = [' LDFLAGS=''$LDFLAGS -fPIC ' rpath ' '' '] ;
        end

        include_flags = sprintf('-I%s', paths.include_dir);
        if ~isempty(paths.hdf5_include_dir)
            include_flags = sprintf('%s -I%s', include_flags, ...
                paths.hdf5_include_dir);
        end
        mex_command = sprintf ('mex %s %s %s %s %s -lhdf5_serial', ...
            include_flags, rpath, lib_path, mex_file, cjson_lib) ;
        if verbose
            mex_command = [mex_command, ' -v'] ;
            fprintf('\n    %s\n', mex_command);
        end

        try
            eval (mex_command) ;
            fprintf('SUCCESS\n');
        catch me
            fprintf('FAILED\n');
            fprintf('    Error: %s\n', me.message);
            failed_files{end+1} = mex_file;
        end
    end

    if ~isempty(failed_files)
        error('Failed to compile MEX file(s): %s', ...
              strjoin(failed_files, ', '));
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
