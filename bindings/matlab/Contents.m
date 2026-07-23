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
%   generate_bsp_from_ssmc          - write an SSMC Problem to a Binsparse file
%   convert_to_problem_struct       - convert a Binsparse problem to an SSMC Problem
%   bsp_matrix_create               - create a Binsparse matrix struct
%   bsp_matrix_info                 - display information about a Binsparse matrix struct
%
% Build scripts:
%   build_matlab_bindings           - build all MEX functions with MATLAB's mex
%   build_octave_bindings           - build all MEX functions with Octave's mkoctfile
%   compile_octave.sh               - build the Octave MEX functions from the shell
%
% Tests:
%   test_binsparse_read             - error-handling tests for binsparse_read
%   test_binsparse_write            - write and round-trip tests for binsparse_write
%   test_binsparse_from_ssmc        - basic test for binsparse_from_ssmc
%   test_binsparse_minimize_roundtrip - SSMC conversion + type minimization test
%   test_bsp_matrix_struct          - tests for the Binsparse matrix struct helpers
%   test_convert_to_problem_struct  - tests for Binsparse Problem conversion
%   test_generate_bsp_from_ssmc     - end-to-end test for generate_bsp_from_ssmc
%   test_binsparse_roundtrip_dir    - round-trip every .h5 file in a directory

% SPDX-FileCopyrightText: 2024 Binsparse Developers
%
% SPDX-License-Identifier: BSD-3-Clause
