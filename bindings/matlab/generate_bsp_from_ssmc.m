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
primary_metadata = metadata_json(P, 'A');
binsparse_write(output_filename, mat, '', primary_metadata, compression_level);

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

end

function handle_aux_entry(name, value, output_filename, format, compression_level)
    if isempty(value)
        return;
    end

    if is_text_value(value)
        write_string_dataset(output_filename, name, value);
        return;
    end

    if iscell(value)
        len = numel(value);
        for k = 1:len
            handle_aux_entry(component_name(name, k, len), value{k}, ...
                             output_filename, format, compression_level);
        end
        return;
    end

    if islogical(value)
        value = double(value);
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
    binsparse_write(output_filename, bsp, name, entry_metadata_json(name), ...
                    compression_level);
end

function fmt = dense_format_for(value)
    if isvector(value)
        fmt = 'DVEC';
    else
        fmt = 'DMAT';
    end
end

function name = component_name(prefix, index, count)
    if count < 10
        pattern = '%s_%d';
    elseif count < 100
        if index < 10
            pattern = '%s_0%d';
        else
            pattern = '%s_%d';
        end
    else
        if index < 10
            pattern = '%s_00%d';
        elseif index < 100
            pattern = '%s_0%d';
        else
            pattern = '%s_%d';
        end
    end
    name = sprintf(pattern, prefix, index);
end

function ok = is_text_value(value)
    ok = ischar(value) || isstring(value) || iscellstr(value);
end

function write_string_dataset(output_filename, name, value)
    if exist('binsparse_write_string_dataset', 'file') ~= 3
        error('generate_bsp_from_ssmc:MissingStringWriter', ...
              'BSP text output requires binsparse_write_string_dataset on the path');
    end

    if isstring(value)
        if isscalar(value)
            value = char(value);
        else
            value = cellstr(value(:));
        end
    elseif ischar(value)
        if size(value, 1) > 1
            value = char_rows(value);
        else
            value = char(value);
        end
    elseif iscellstr(value)
        value = value(:);
    else
        error('generate_bsp_from_ssmc:InvalidStringValue', ...
              'Text aux value must be char, string, or cellstr');
    end

    binsparse_write_string_dataset(output_filename, name, value);
end

function json = metadata_json(P, role)
    metadata_fields = {'name', 'title', 'id', 'date', 'author', 'ed', ...
                       'editor', 'kind', 'notes', 'group', 'num_rows', ...
                       'num_cols', 'nnz', 'pattern_symmetry', ...
                       'numerical_symmetry', 'type', 'structure'};
    metadata = struct();
    metadata.role = role;

    for i = 1:numel(metadata_fields)
        field = metadata_fields{i};
        if isfield(P, field)
            [ok, value] = metadata_value(P.(field));
            if ok
                metadata.(field) = value;
            end
        end
    end

    json = metadata_payload_json(metadata);
end

function json = entry_metadata_json(role)
    metadata = struct();
    metadata.role = role;
    json = metadata_payload_json(metadata);
end

function json = metadata_payload_json(metadata)
    payload = struct();
    payload.metadata = metadata;
    json = encode_json_or_empty(payload);
end

function [ok, value] = metadata_value(value)
    ok = false;
    if ischar(value)
        if size(value, 1) > 1
            value = char_rows(value);
        else
            value = char(value);
        end
        ok = true;
    elseif isstring(value)
        if isscalar(value)
            value = char(value);
        else
            value = cellstr(value(:));
        end
        ok = true;
    elseif (islogical(value) || isnumeric(value)) && numel(value) <= 64
        ok = true;
    end
end

function rows = char_rows(value)
    rows = cell(size(value, 1), 1);
    for k = 1:size(value, 1)
        rows{k} = value(k, :);
    end
end

function json = encode_json_or_empty(value)
    try
        json = jsonencode(value);
    catch
        json = [];
    end
end
