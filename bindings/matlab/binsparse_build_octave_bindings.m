function binsparse_build_octave_bindings(varargin)
% BINSPARSE_BUILD_OCTAVE_BINDINGS - Build Binsparse Octave MEX functions
%
% This script provides a simple interface to build Octave bindings
% for the Binsparse library using mkoctfile. It automatically detects
% include paths and sets up the compilation environment.
%
% Usage:
%   binsparse_build_octave_bindings()           % Build all available MEX functions
%   binsparse_build_octave_bindings('verbose')  % Build with verbose output
%   binsparse_build_octave_bindings('clean')    % Clean compiled MEX files
%
% Prerequisites:
% - GNU Octave with mkoctfile (usually included with Octave)
% - Binsparse C library headers (in ../../include/)
% - C compiler (gcc recommended)
%
% Note: This script builds Octave-compatible MEX functions using mkoctfile
%       instead of MATLAB's mex command.

% SPDX-FileCopyrightText: 2024 Binsparse Developers
%
% SPDX-License-Identifier: BSD-3-Clause

% Parse input arguments
verbose = any(strcmpi(varargin, 'verbose'));
clean_only = any(strcmpi(varargin, 'clean'));

fprintf('=== Binsparse Octave Bindings Build Script ===\n\n');

if clean_only
    fprintf('Cleaning compiled MEX files...\n');
    clean_mex_files();
    return;
end

% Check if we're running in Octave
if ~is_octave()
    warning(['This script is designed for Octave. For MATLAB, use ' ...
             'binsparse_build_matlab_bindings.m']);
end

% Check mkoctfile availability
if ~check_mkoctfile()
    error('mkoctfile not found. Please ensure Octave is properly installed.');
end

% Find and validate paths
paths = get_build_paths();
if verbose
    fprintf('Build paths:\n');
    fprintf('  Current dir: %s\n', paths.current_dir);
    fprintf('  Include dir: %s\n', paths.include_dir);
    if ~isempty(paths.hdf5_include_dir)
        fprintf('  HDF5 include dir: %s\n', paths.hdf5_include_dir);
    end
    fprintf('  Root dir: %s\n', paths.binsparse_root);
    fprintf('\n');
end

% Compile MEX functions
compile_octave_functions(paths, verbose);

fprintf('\n=== Build Complete ===\n');
fprintf('Run the test functions to verify the installation:\n');
fprintf('  test_binsparse_read()\n');
fprintf('  test_binsparse_write()\n');
fprintf('  test_binsparse_write_ssmc_problem()\n\n');

end

function result = is_octave()
    % Check if running in Octave
    result = exist('OCTAVE_VERSION', 'builtin') ~= 0;
end

function success = check_mkoctfile()
    % Check if mkoctfile is available
    try
        [status, ~] = system('mkoctfile --version');
        success = (status == 0);
        if success && nargout == 0
            fprintf('mkoctfile found and working\n');
        end
    catch
        success = false;
    end
end

function paths = get_build_paths()
    % Get and validate build paths
    paths.current_dir = pwd;
    paths.binsparse_root = fullfile(paths.current_dir, '..', '..');
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

function compile_octave_functions(paths, verbose)
    % Compile all MEX functions using mkoctfile

    % List of MEX functions to compile
    mex_files = {'binsparse_read.c', 'binsparse_write.c', ...
        'binsparse_from_ssmc.c', 'binsparse_minimize_types.c', ...
        'binsparse_write_string_dataset.c'};

    fprintf('Compiling MEX functions with mkoctfile...\n');
    failed_files = {};

    for i = 1:length(mex_files)
        mex_file = mex_files{i};
        if ~exist(mex_file, 'file')
            warning('MEX source file not found: %s', mex_file);
            continue;
        end

        fprintf('  Compiling %s... ', mex_file);

        % Prepare mkoctfile command with library linking
        include_flag = sprintf('-I%s', paths.include_dir);
        if ~isempty(paths.hdf5_include_dir)
            include_flag = sprintf('%s -I%s', include_flag, ...
                paths.hdf5_include_dir);
        end
        lib_dir = fullfile(paths.binsparse_root, 'build');
        lib_path = fullfile(lib_dir, 'libbinsparse.a');
        cjson_lib_dir = fullfile(lib_dir, '_deps', 'cjson-build');

        if verbose
            cmd = sprintf('mkoctfile --mex --verbose -fPIC %s %s -Wl,--whole-archive %s -Wl,--no-whole-archive -L%s -lcjson -lhdf5_serial', ...
                         include_flag, mex_file, lib_path, cjson_lib_dir);
        else
            cmd = sprintf('mkoctfile --mex -fPIC %s %s -Wl,--whole-archive %s -Wl,--no-whole-archive -L%s -lcjson -lhdf5_serial', ...
                         include_flag, mex_file, lib_path, cjson_lib_dir);
        end

        if verbose
            fprintf('\n    Command: %s\n', cmd);
        end

        % Execute mkoctfile
        [status, output] = system(cmd);

        if status == 0
            fprintf('SUCCESS\n');
            if verbose && ~isempty(output)
                fprintf('    Output: %s\n', output);
            end
        else
            fprintf('FAILED\n');
            fprintf('    Error output:\n%s\n', output);
            failed_files{end+1} = mex_file;
        end
    end

    if ~isempty(failed_files)
        error('Failed to compile MEX file(s): %s', ...
              strjoin(failed_files, ', '));
    end
end

function clean_mex_files()
    % Clean compiled MEX files (Octave uses different extensions)

    % Octave MEX extensions vary by platform
    if ispc
        extensions = {'mexw32', 'mexw64'};
    elseif ismac
        extensions = {'mexmaci64'};
    else
        extensions = {'mexa64', 'mex'};
    end

    found_files = {};

    % Find files with any of the MEX extensions
    for i = 1:length(extensions)
        ext = extensions{i};
        files = dir(['*.' ext]);
        for j = 1:length(files)
            found_files{end+1} = files(j).name;
        end
    end

    if isempty(found_files)
        fprintf('No MEX files found to clean.\n');
        return;
    end

    fprintf('Removing %d MEX file(s):\n', length(found_files));
    for i = 1:length(found_files)
        fprintf('  %s\n', found_files{i});
        delete(found_files{i});
    end
end
