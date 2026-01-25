% SPDX-FileCopyrightText: 2024 Binsparse Developers
%
% SPDX-License-Identifier: BSD-3-Clause

function matrix = binsparse_from_ssmc (A, Zeros, format)
%BINSPARSE_FROM_SSMC convert SuiteSparse A+Zeros to a Binsparse matrix struct
%
% Usage:
%   matrix = binsparse_from_ssmc(A, Zeros)
%   matrix = binsparse_from_ssmc(A, Zeros, format)
%
% This is a thin wrapper over the binsparse_from_ssmc MEX function.

error('binsparse_from_ssmc mexFunction not found; compile with build_matlab_bindings first');
