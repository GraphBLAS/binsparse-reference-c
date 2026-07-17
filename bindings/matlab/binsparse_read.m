function matrix = binsparse_read (filename, group)
%BINSPARSE_READ read a matrix from a Binsparse HDF5 file
%
% Usage:
%   matrix = binsparse_read (filename)          % read from the file root
%   matrix = binsparse_read (filename, group)   % read from an HDF5 group
%
% filename is the path of a Binsparse HDF5 file (typically *.bsp.h5), and
% group optionally names an HDF5 group within the file that holds a
% Binsparse matrix.  The result is a raw Binsparse matrix struct with the
% fields values, indices_0, indices_1, pointers_to_1, nrows, ncols, nnz,
% is_iso, format, and structure, as documented in bsp_matrix_create.
% Value and index arrays keep the types stored in the file; indices are
% 0-based.  Use convert_to_problem_struct to build a MATLAB-native
% SuiteSparse Problem struct from Binsparse data.
%
% Example:
%   matrix = binsparse_read ('west0067.bsp.h5') ;
%   b = binsparse_read ('west0067.bsp.h5', 'b') ;
%
% See also binsparse_write, bsp_matrix_create, convert_to_problem_struct.

% SPDX-FileCopyrightText: 2024 Binsparse Developers
%
% SPDX-License-Identifier: BSD-3-Clause

% This .m file provides the help text for the binsparse_read MEX function.

error ('binsparse_read mexFunction not found; compile with build_matlab_bindings first') ;
