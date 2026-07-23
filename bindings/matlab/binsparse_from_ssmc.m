function matrix = binsparse_from_ssmc (A, Zeros, format)
%BINSPARSE_FROM_SSMC convert SuiteSparse A+Zeros to a Binsparse matrix struct
%
% Usage:
%   matrix = binsparse_from_ssmc (A)                  % default format
%   matrix = binsparse_from_ssmc (A, format)
%   matrix = binsparse_from_ssmc (A, Zeros)
%   matrix = binsparse_from_ssmc (A, Zeros, format)
%
% A is a sparse or dense MATLAB matrix.  Zeros is an optional sparse matrix
% of the same size as A holding the pattern of explicit zero entries (as in
% a SuiteSparse Matrix Collection Problem.Zeros); its entries are stored in
% the output with the value zero.  format is an optional format string:
% 'COO' (the default), 'COOR', 'CSC', or 'CSR' for sparse input, or
% 'DVEC', 'DMAT', or 'DMATC' for dense input.  The result is a Binsparse
% matrix struct suitable for binsparse_write.
%
% Example:
%   A = sparse ([1 3], [1 2], [5 7], 3, 3) ;
%   Zeros = sparse (2, 3, 1, 3, 3) ;
%   matrix = binsparse_from_ssmc (A, Zeros, 'CSC') ;
%
% See also binsparse_write, binsparse_minimize_types,
% binsparse_write_ssmc_problem.

% SPDX-FileCopyrightText: 2024 Binsparse Developers
%
% SPDX-License-Identifier: BSD-3-Clause

% This .m file provides the help text for the binsparse_from_ssmc MEX
% function.

error(['binsparse_from_ssmc mexFunction not found; compile with ' ...
       'binsparse_build_matlab_bindings first']);
