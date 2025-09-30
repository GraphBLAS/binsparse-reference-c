% SPDX-FileCopyrightText: 2024 Binsparse Developers
%
% SPDX-License-Identifier: BSD-3-Clause

function compile_write_binsparse_from_matlab_octave(varargin)
% COMPILE_WRITE_BINSPARSE_FROM_MATLAB_OCTAVE - Compile the write_binsparse_from_matlab MEX function for Octave
%
% This script compiles the write_binsparse_from_matlab MEX function for Octave using mkoctfile.
% It automatically detects include paths and links against the Binsparse library.
%
% Usage:
%   compile_write_binsparse_from_matlab_octave()           % Standard compilation
%   compile_write_binsparse_from_matlab_octave('verbose')  % Verbose compilation output
%
% Prerequisites:
% - GNU Octave with mkoctfile
% - Binsparse C library headers (in ../../include/)
% - Compiled Binsparse library (in ../../build/)

% Parse input arguments
verbose = any(strcmpi(varargin, 'verbose'));

fprintf('=== Compiling write_binsparse_from_matlab MEX function for Octave ===\n\n');

% Check if we're running in Octave
if ~is_octave()
    warning('This script is designed for Octave. For MATLAB, use compile_write_binsparse_from_matlab.m');
end

% Check if source file exists
source_file = 'write_binsparse_from_matlab.c';
if ~exist(source_file, 'file')
    error('Source file not found: %s\nEnsure you are in the correct directory.', source_file);
end

% Check mkoctfile availability
if ~check_mkoctfile()
    error('mkoctfile not found. Please ensure Octave is properly installed.');
end

% Get build paths
paths = get_build_paths();
if verbose
    fprintf('Build paths:\n');
    fprintf('  Current dir: %s\n', paths.current_dir);
    fprintf('  Include dir: %s\n', paths.include_dir);
    fprintf('  Library dir: %s\n', paths.lib_dir);
    fprintf('  Root dir: %s\n', paths.binsparse_root);
    fprintf('\n');
end

% Compile the MEX function
compile_octave_function(source_file, paths, verbose);

fprintf('\n=== Compilation Complete ===\n');
fprintf('Test the function with:\n');
fprintf('  test_write_binsparse_from_matlab()\n\n');

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

function compile_octave_function(source_file, paths, verbose)
    % Compile the MEX function using mkoctfile

    fprintf('Compiling %s with mkoctfile... ', source_file);

    % Prepare mkoctfile command with library linking
    include_flag = sprintf('-I%s', paths.include_dir);
    lib_path = fullfile(paths.lib_dir, 'libbinsparse.a');
    cjson_lib_dir = fullfile(paths.lib_dir, '_deps', 'cjson-build');

    if verbose
        cmd = sprintf('mkoctfile --mex --verbose -fPIC %s %s -Wl,--whole-archive %s -Wl,--no-whole-archive -L%s -lcjson -lhdf5_serial', ...
                     include_flag, source_file, lib_path, cjson_lib_dir);
    else
        cmd = sprintf('mkoctfile --mex -fPIC %s %s -Wl,--whole-archive %s -Wl,--no-whole-archive -L%s -lcjson -lhdf5_serial', ...
                     include_flag, source_file, lib_path, cjson_lib_dir);
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

        % Provide troubleshooting suggestions
        fprintf('\nTroubleshooting suggestions:\n');
        fprintf('  1. Ensure the Binsparse library has been built: cd ../../build && make\n');
        fprintf('  2. Check that Octave development packages are installed\n');
        fprintf('  3. Verify HDF5 development libraries are installed\n');
        fprintf('  4. Try running with verbose flag for more details\n');

        error('Compilation failed');
    end
end
