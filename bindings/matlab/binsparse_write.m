function binsparse_write(filename, matrix, group, json_string, compression_level)
%BINSPARSE_WRITE write a matrix to a file in Binsparse HDF5 format
%
% Usage:
%   binsparse_write (filename, matrix)
%   binsparse_write (filename, matrix, group)
%   binsparse_write (filename, matrix, group, json_string)
%   binsparse_write (filename, matrix, group, json_string, compression_level)
%
% filename is the path of the Binsparse HDF5 file to create (typically
% *.bsp.h5), and matrix is a Binsparse matrix struct as returned by
% binsparse_read, binsparse_from_ssmc, or bsp_matrix_create.  The optional
% group names an HDF5 group to write into ('' or [ ] writes to the file
% root).  The optional json_string is a JSON object whose keys are merged
% into the Binsparse descriptor as user metadata.  The optional
% compression_level selects gzip compression from 0 (none) to 9; the
% default is 1.
%
% Example:
%   matrix = binsparse_from_ssmc (sparse ([1 2], [1 2], [3 4])) ;
%   binsparse_write ('example.bsp.h5', matrix) ;
%   binsparse_write ('example.bsp.h5', matrix, 'b', '{"role": "b"}', 9) ;
%
% See also binsparse_read, binsparse_from_ssmc, bsp_matrix_create,
% generate_bsp_from_ssmc.

% SPDX-FileCopyrightText: 2024 Binsparse Developers
% SPDX-License-Identifier: BSD-3-Clause

% This .m file provides the help text for the binsparse_write MEX function.

error ('binsparse_write mexFunction not found; compile with build_matlab_bindings first') ;
