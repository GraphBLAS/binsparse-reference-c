% SPDX-FileCopyrightText: 2024 Binsparse Developers
%
% SPDX-License-Identifier: BSD-3-Clause

function generate_bsp_from_ssmc(problem, output_filename, format, compression_level)
% GENERATE_BSP_FROM_SSMC - Generate a Binsparse file from an SSMC problem
%
% Usage:
%   generate_bsp_from_ssmc(problem, output_filename)
%   generate_bsp_from_ssmc(problem, output_filename, format, compression_level)
%
% Defaults:
%   format = 'COO'
%   compression_level = 0

if nargin < 2
    error('generate_bsp_from_ssmc:InvalidArgs', ...
          'Usage: generate_bsp_from_ssmc(problem, output_filename [, format [, compression_level]])');
end

if nargin < 3 || isempty(format)
    format = 'COO';
end

if nargin < 4 || isempty(compression_level)
    compression_level = 0;
end

if ~isstruct(problem)
    error('generate_bsp_from_ssmc:InvalidProblem', 'Problem must be a struct');
end

if isfield(problem, 'Problem')
    P = problem.Problem;
else
    P = problem;
end

if ~isfield(P, 'A')
    error('generate_bsp_from_ssmc:MissingMatrix', 'Problem.A is required');
end

% Primary matrix
A = P.A;
if issparse(A)
    if isfield(P, 'Zeros') && ~isempty(P.Zeros)
        Zeros = P.Zeros;
        mat = binsparse_from_ssmc(A, Zeros, format);
    else
        mat = binsparse_from_ssmc(A, format);
    end
else
    mat = binsparse_from_ssmc(A, dense_format_for(A));
end

mat = binsparse_minimize_types(mat);
binsparse_write(output_filename, mat, '', [], compression_level);

% Handle aux struct
if isfield(P, 'aux') && isstruct(P.aux)
    aux_names = fieldnames(P.aux);
    for i = 1:numel(aux_names)
        name = aux_names{i};
        value = P.aux.(name);
        handle_aux_entry(name, value, output_filename, format, compression_level);
    end
end

% Handle x and b like aux
if isfield(P, 'x') && ~isempty(P.x)
    handle_aux_entry('x', P.x, output_filename, format, compression_level);
end

if isfield(P, 'b') && ~isempty(P.b)
    handle_aux_entry('b', P.b, output_filename, format, compression_level);
end

% Metadata handling not yet implemented
fprintf('Note: metadata handling is not implemented yet.\n');

end

function handle_aux_entry(name, value, output_filename, format, compression_level)
    if ischar(value) || (isstring(value) && isscalar(value))
        fprintf('Note: aux entry "%s" is a string and is currently ignored.\n', name);
        return;
    end

    if issparse(value)
        bsp = binsparse_from_ssmc(value, format);
    elseif isnumeric(value)
        bsp = binsparse_from_ssmc(value, dense_format_for(value));
    else
        fprintf('Note: aux entry "%s" has unsupported type and is ignored.\n', name);
        return;
    end

    bsp = binsparse_minimize_types(bsp);
    binsparse_write(output_filename, bsp, name, [], compression_level);
end

function fmt = dense_format_for(value)
    if isvector(value)
        fmt = 'DVEC';
    else
        fmt = 'DMAT';
    end
end
