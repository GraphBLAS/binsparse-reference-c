% SPDX-FileCopyrightText: 2024 Binsparse Developers
%
% SPDX-License-Identifier: BSD-3-Clause

function bsp_matrix_info(matrix)
% BSP_MATRIX_INFO - Display information about a Binsparse matrix struct
%
% Usage:
%   bsp_matrix_info(matrix)
%
% Displays:
%   - Matrix dimensions and number of non-zeros
%   - Format and structure information
%   - Array sizes and types for each field

if ~isstruct(matrix)
    error('bsp_matrix_info:InvalidInput', 'Input must be a struct');
end

% Display basic matrix information
fprintf('%lu x %lu matrix with %lu nnz\n', ...
        matrix.nrows, matrix.ncols, matrix.nnz);
fprintf('%s format with %s structure\n', ...
        matrix.format, matrix.structure);

if matrix.is_iso
    fprintf('ISO matrix (single value)\n');
end

% Display array information
if ~isempty(matrix.values)
    fprintf('%lu values of type %s\n', ...
            length(matrix.values), class(matrix.values));
end

if ~isempty(matrix.indices_0)
    fprintf('%lu indices_0 of type %s\n', ...
            length(matrix.indices_0), class(matrix.indices_0));
end

if ~isempty(matrix.indices_1)
    fprintf('%lu indices_1 of type %s\n', ...
            length(matrix.indices_1), class(matrix.indices_1));
end

if ~isempty(matrix.pointers_to_1)
    fprintf('%lu pointers_to_1 of type %s\n', ...
            length(matrix.pointers_to_1), class(matrix.pointers_to_1));
end

end
