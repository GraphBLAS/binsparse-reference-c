% Binsparse MATLAB/Octave bindings
%
% Core functions (MEX, with .m help stubs):
%   binsparse_read                  - read a matrix from a Binsparse HDF5 file
%   binsparse_write                 - write a matrix to a Binsparse HDF5 file
%   binsparse_from_ssmc             - convert SSMC A+Zeros to a Binsparse matrix struct
%   binsparse_minimize_types        - minimize value/index types in a Binsparse struct
%   binsparse_write_string_dataset  - write an HDF5 UTF-8 string dataset
%
% MATLAB helpers:
%   binsparse_write_ssmc_problem    - write an SSMC Problem to a Binsparse file
%   binsparse_to_ssmc_problem       - convert a Binsparse problem to an SSMC Problem
%   binsparse_create_struct         - create a Binsparse vector or matrix struct
%   binsparse_info                  - display information about a Binsparse struct
%
% Build scripts:
%   binsparse_build_matlab_bindings - build all MEX functions with MATLAB's mex
%   binsparse_build_octave_bindings - build all MEX functions with Octave's mkoctfile
%   compile_octave.sh               - build the Octave MEX functions from the shell
%
% Tests:
%   test_binsparse_read             - error-handling tests for binsparse_read
%   test_binsparse_write            - write and round-trip tests for binsparse_write
%   test_binsparse_from_ssmc        - basic test for binsparse_from_ssmc
%   test_binsparse_minimize_roundtrip - SSMC conversion + type minimization test
%   test_binsparse_struct           - tests for the Binsparse struct helpers
%   test_binsparse_to_ssmc_problem  - tests for Binsparse Problem conversion
%   test_binsparse_write_ssmc_problem - end-to-end SSMC writer test
%   test_binsparse_roundtrip_dir    - round-trip every .h5 file in a directory

% SPDX-FileCopyrightText: 2024 Binsparse Developers
%
% SPDX-License-Identifier: BSD-3-Clause
