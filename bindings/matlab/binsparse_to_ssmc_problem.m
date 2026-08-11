function Problem = binsparse_to_ssmc_problem(bsp_problem)
%BINSPARSE_TO_SSMC_PROBLEM convert a Binsparse problem to an SSMC Problem
%
% Problem = binsparse_to_ssmc_problem(bsp_problem)
%
% bsp_problem is an in-memory representation of one SuiteSparse Matrix
% Collection problem.  Its A, b, x, and aux numeric entries are raw structs
% returned by binsparse_read.  The metadata field contains the user metadata
% from the root Binsparse JSON descriptor.  Text entries are the char matrices
% and cellstr values returned by binsparse_read_string_dataset, which recovers
% the MATLAB class from the HDF5 string datatype.

% SPDX-FileCopyrightText: 2026 Binsparse Developers
%
% SPDX-License-Identifier: BSD-3-Clause

validate_problem(bsp_problem);

Problem = metadata_to_problem(bsp_problem.metadata);
[Problem.A, Zeros] = convert_matrix(bsp_problem.A, true);
if ~issparse(Problem.A) || any(size(Problem.A) == 0)
    error('BinSparse:InvalidProblem', ...
          'The primary SSMC matrix must be sparse and nonempty');
end
if nnz(Zeros) > 0
    Problem.Zeros = Zeros;
end

if isfield(bsp_problem, 'b')
    Problem.b = convert_component(bsp_problem.b);
end
if isfield(bsp_problem, 'x')
    Problem.x = convert_component(bsp_problem.x);
end
if isfield(bsp_problem, 'aux')
    Problem.aux = convert_aux(bsp_problem.aux);
end

end

function validate_problem(bsp_problem)
if ~isstruct(bsp_problem) || ~isscalar(bsp_problem)
    error('BinSparse:InvalidProblem', ...
          'Binsparse problem must be a scalar struct');
end
if ~isfield(bsp_problem, 'metadata') || ...
        ~isstruct(bsp_problem.metadata) || ~isscalar(bsp_problem.metadata)
    error('BinSparse:InvalidProblem', ...
          'Binsparse problem must contain scalar metadata');
end
if ~isfield(bsp_problem, 'A') || ~is_bsp_matrix(bsp_problem.A)
    error('BinSparse:InvalidProblem', ...
          'Binsparse problem must contain a raw primary matrix A');
end
end

function Problem = metadata_to_problem(metadata)
Problem = struct();
names = fieldnames(metadata);
for k = 1:numel(names)
    name = names{k};
    if strcmp(name, 'role')
        continue;
    end
    value = metadata.(name);
    if strcmp(name, 'notes')
        value = text_rows(value);
    elseif any(strcmp(name, {'name', 'title', 'date', 'author', 'ed', ...
                             'editor', 'kind', 'group', 'type', ...
                             'structure'}))
        value = text_scalar(value, name);
    end
    Problem.(name) = value;
end

required = {'name', 'title', 'id', 'date', 'author', 'ed', 'kind'};
for k = 1:numel(required)
    if ~isfield(Problem, required{k})
        error('BinSparse:InvalidMetadata', ...
              'Missing required SSMC metadata field: %s', required{k});
    end
end
end

function value = convert_component(value)
if is_bsp_matrix(value)
    value = convert_matrix(value, false);
elseif is_text(value)
    value = normalize_component_text(value);
else
    error('BinSparse:InvalidComponent', ...
          'Unsupported Binsparse problem component');
end
end

function aux = convert_aux(raw_aux)
if ~isstruct(raw_aux) || ~isscalar(raw_aux)
    error('BinSparse:InvalidAux', 'aux must be a scalar struct');
end

aux = struct();
names = fieldnames(raw_aux);
for k = 1:numel(names)
    name = names{k};
    value = convert_component(raw_aux.(name));
    tokens = regexp(name, '^(.*)_([0-9]+)$', 'tokens', 'once');
    if isempty(tokens) || isempty(tokens{1})
        if isfield(aux, name)
            error('BinSparse:DuplicateComponent', ...
                  'Duplicate auxiliary component: %s', name);
        end
        aux.(name) = value;
        continue;
    end

    base = tokens{1};
    index = str2double(tokens{2});
    if ~isfinite(index) || index < 1 || fix(index) ~= index
        error('BinSparse:InvalidComponent', ...
              'Invalid auxiliary cell component: %s', name);
    end
    if isfield(aux, base) && ~iscell(aux.(base))
        error('BinSparse:DuplicateComponent', ...
              'Auxiliary component conflicts with cell sequence: %s', base);
    end
    if ~isfield(aux, base)
        aux.(base) = cell(0, 1);
    end
    if numel(aux.(base)) >= index && ~isempty(aux.(base){index})
        error('BinSparse:DuplicateComponent', ...
              'Duplicate auxiliary cell component: %s', name);
    end
    aux.(base){index, 1} = value;
end
end

function [matrix, Zeros] = convert_matrix(bsp, split_zeros)
validate_matrix_header(bsp);
format = upper(char(bsp.format));
m = checked_size(bsp.nrows, 'nrows');
n = checked_size(bsp.ncols, 'ncols');
count = checked_size(bsp.nnz, 'nnz');

switch format
    case 'DVEC'
        [values, iso] = matrix_values(bsp, count);
        if n ~= 1 || count ~= m
            error('BinSparse:InvalidMatrix', ...
                  'DVEC dimensions do not match the stored value count');
        end
        matrix = reshape(expand_iso(values, iso, count), [m, 1]);
        Zeros = sparse(m, n);
        return;

    case {'DMAT', 'DMATR'}
        expected = checked_product(m, n);
        if count ~= expected
            error('BinSparse:InvalidMatrix', ...
                  'DMATR dimensions do not match the stored value count');
        end
        [values, iso] = matrix_values(bsp, count);
        matrix = reshape(expand_iso(values, iso, count), [n, m]).';
        Zeros = sparse(m, n);
        return;

    case 'DMATC'
        expected = checked_product(m, n);
        if count ~= expected
            error('BinSparse:InvalidMatrix', ...
                  'DMATC dimensions do not match the stored value count');
        end
        [values, iso] = matrix_values(bsp, count);
        matrix = reshape(expand_iso(values, iso, count), [m, n]);
        Zeros = sparse(m, n);
        return;
end

[rows, cols] = sparse_indices(bsp, format, m, n, count);
[values, iso] = matrix_values(bsp, count);
[rows, cols, values, iso] = ...
    expand_structure(bsp, rows, cols, values, iso, m, n);

% An ISO matrix carries a single stored value, and sparse() expands a scalar
% over the index vectors, so its values are never materialized at nnz length.
% The entries are then either all zero or all nonzero, which also removes the
% need for a mask and for masked copies of the index vectors.
if iso
    if split_zeros && values == 0
        matrix = sparse(m, n);
        Zeros = sparse(rows, cols, 1, m, n);
    else
        matrix = sparse(rows, cols, values, m, n);
        Zeros = sparse(m, n);
    end
    return;
end

if ~split_zeros
    matrix = sparse(rows, cols, values, m, n);
    Zeros = sparse(m, n);
    return;
end

% MATLAB's == compares both parts of a complex value, so this needs no
% separate imag() test; taking one would allocate an nnz-length array of
% zeros for the common case of real values.
is_zero = (values == 0);
if ~any(is_zero)
    matrix = sparse(rows, cols, values, m, n);
    Zeros = sparse(m, n);
else
    keep = ~is_zero;
    matrix = sparse(rows(keep), cols(keep), values(keep), m, n);
    clear keep
    Zeros = sparse(rows(is_zero), cols(is_zero), 1, m, n);
end
end

function [rows, cols] = sparse_indices(bsp, format, m, n, count)
switch format
    case {'COO', 'COOR'}
        rows = checked_indices(bsp.indices_0, count, m, 'indices_0') + 1;
        cols = checked_indices(bsp.indices_1, count, n, 'indices_1') + 1;
        require_ordered_pairs(rows, cols, 'COOR');

    case 'COOC'
        cols = checked_indices(bsp.indices_0, count, n, 'indices_0') + 1;
        rows = checked_indices(bsp.indices_1, count, m, 'indices_1') + 1;
        require_ordered_pairs(cols, rows, 'COOC');

    case 'CSR'
        pointers = checked_pointers(bsp.pointers_to_1, m + 1, count);
        cols = checked_indices(bsp.indices_1, count, n, 'indices_1') + 1;
        rows = expand_major_indices((0:m-1).', diff(pointers)) + 1;
        require_segment_order(cols, pointers, 'CSR');

    case 'CSC'
        pointers = checked_pointers(bsp.pointers_to_1, n + 1, count);
        rows = checked_indices(bsp.indices_1, count, m, 'indices_1') + 1;
        cols = expand_major_indices((0:n-1).', diff(pointers)) + 1;
        require_segment_order(rows, pointers, 'CSC');

    case 'DCSR'
        major = checked_indices(bsp.indices_0, [], m, 'indices_0');
        require_strictly_increasing(major, 'DCSR major indices');
        pointers = checked_pointers(bsp.pointers_to_1, numel(major) + 1, count);
        require_nonempty_segments(pointers, 'DCSR');
        cols = checked_indices(bsp.indices_1, count, n, 'indices_1') + 1;
        rows = expand_major_indices(major, diff(pointers)) + 1;
        require_segment_order(cols, pointers, 'DCSR');

    case 'DCSC'
        major = checked_indices(bsp.indices_0, [], n, 'indices_0');
        require_strictly_increasing(major, 'DCSC major indices');
        pointers = checked_pointers(bsp.pointers_to_1, numel(major) + 1, count);
        require_nonempty_segments(pointers, 'DCSC');
        rows = checked_indices(bsp.indices_1, count, m, 'indices_1') + 1;
        cols = expand_major_indices(major, diff(pointers)) + 1;
        require_segment_order(rows, pointers, 'DCSC');

    case 'CVEC'
        if n ~= 1
            error('BinSparse:InvalidMatrix', ...
                  'CVEC must have one MATLAB column');
        end
        rows = checked_indices(bsp.indices_0, count, m, 'indices_0') + 1;
        require_strictly_increasing(rows, 'CVEC indices');
        cols = ones(count, 1);

    otherwise
        error('BinSparse:UnsupportedFormat', ...
              'Unsupported Binsparse matrix format: %s', format);
end
end

function [values, iso] = matrix_values(bsp, count)
% Returns the stored values and whether they are still in ISO form, that is a
% single value standing for all count entries.  The caller expands it only
% where an nnz-length array is genuinely required; the sparse constructors
% take the scalar directly.
values = bsp.values(:);
iso = false;
if logical(bsp.is_iso)
    if count == 0
        if numel(values) > 1
            error('BinSparse:InvalidMatrix', ...
                  'An empty ISO matrix has too many values');
        end
        values = values([]);
    elseif numel(values) ~= 1
        error('BinSparse:InvalidMatrix', ...
              'An ISO matrix must contain exactly one value');
    else
        iso = true;
    end
elseif numel(values) ~= count
    error('BinSparse:InvalidMatrix', ...
          'Value array length does not match nnz');
end
values = exact_double(values);
end

function values = expand_iso(values, iso, count)
if iso
    values = repmat(values, count, 1);
end
end

function [rows, cols, values, iso] = ...
    expand_structure(bsp, rows, cols, values, iso, m, n)
structure = 'general';
if isfield(bsp, 'structure') && ~isempty(bsp.structure)
    structure = lower(char(bsp.structure));
end
if strcmp(structure, 'general')
    return;
end
if m ~= n
    error('BinSparse:InvalidStructure', ...
          'A structured Binsparse matrix must be square');
end

is_lower = ends_with(structure, '_lower');
is_upper = ends_with(structure, '_upper');
if ~is_lower && ~is_upper
    error('BinSparse:UnsupportedStructure', ...
          'Unsupported Binsparse matrix structure: %s', structure);
end
if (is_lower && any(rows < cols)) || (is_upper && any(rows > cols))
    error('BinSparse:InvalidStructure', ...
          'Stored entries do not match the declared matrix triangle');
end

if starts_with(structure, 'hermitian_')
    mirror = @conj;
elseif starts_with(structure, 'skew_symmetric_')
    mirror = @uminus;
elseif starts_with(structure, 'symmetric_')
    mirror = @(v) v;
else
    error('BinSparse:UnsupportedStructure', ...
          'Unsupported Binsparse matrix structure: %s', structure);
end

off_diagonal = (rows ~= cols);
mirror_rows = cols(off_diagonal);
mirror_cols = rows(off_diagonal);

% Mirroring an ISO matrix leaves it ISO whenever it leaves the single stored
% value alone, which covers every symmetric matrix and the real hermitian and
% all-zero skew-symmetric ones.  Only the rest have to be expanded here.
if iso && isequaln(mirror(values), values)
    rows = [rows; mirror_rows];
    cols = [cols; mirror_cols];
    return;
end

values = expand_iso(values, iso, numel(rows));
iso = false;
mirror_values = mirror(values(off_diagonal));
rows = [rows; mirror_rows];
cols = [cols; mirror_cols];
values = [values; mirror_values];
end

function validate_matrix_header(bsp)
if ~is_bsp_matrix(bsp) || ~isscalar(bsp)
    error('BinSparse:InvalidMatrix', ...
          'Expected a scalar raw Binsparse matrix struct');
end
if ~isscalar(bsp.is_iso) || ...
        (~islogical(bsp.is_iso) && ~any(bsp.is_iso == [0 1]))
    error('BinSparse:InvalidMatrix', 'is_iso must be a logical scalar');
end
end

function values = exact_double(values)
converted = double(values);
if isinteger(values) && ~isequal(cast(converted, class(values)), values)
    error('BinSparse:InexactValue', ...
          'Integer values cannot be represented exactly in MATLAB sparse form');
end
values = converted;
end

function ok = is_bsp_matrix(value)
required = {'values', 'indices_0', 'indices_1', 'pointers_to_1', ...
            'nrows', 'ncols', 'nnz', 'is_iso', 'format'};
ok = isstruct(value) && isscalar(value) && ...
     all(isfield(value, required));
end

function value = checked_size(value, name)
if ~isnumeric(value) || ~isreal(value) || ~isscalar(value)
    error('BinSparse:InvalidMatrix', '%s must be a real numeric scalar', name);
end
value = double(value);
if ~isfinite(value) || value < 0 || fix(value) ~= value || value > flintmax
    error('BinSparse:InvalidMatrix', 'Invalid %s value', name);
end
end

function value = checked_product(a, b)
value = a * b;
if ~isfinite(value) || value > flintmax || fix(value) ~= value
    error('BinSparse:InvalidMatrix', 'Dense matrix size is too large');
end
end

function indices = checked_indices(indices, expected, limit, name)
if ~isnumeric(indices) || ~isreal(indices)
    error('BinSparse:InvalidMatrix', '%s must be real numeric data', name);
end
if ~isempty(expected) && numel(indices) ~= expected
    error('BinSparse:InvalidMatrix', ...
          '%s length does not match nnz', name);
end

if isinteger(indices)
    % A Binsparse index array is normally stored as an integer type, where
    % the finiteness and integrality tests are vacuous and the range tests
    % reduce to min and max.  Running them here, before the widening to
    % double, keeps the check from building a second nnz-length temporary
    % just to hold fix(indices).
    if ~isempty(indices)
        if double(max(indices(:))) >= limit || ...
                (intmin(class(indices)) < 0 && double(min(indices(:))) < 0)
            error('BinSparse:InvalidMatrix', ...
                  '%s contains an invalid index', name);
        end
    end
    indices = double(indices(:));
else
    indices = double(indices(:));
    if any(~isfinite(indices)) || any(indices < 0) || ...
            any(fix(indices) ~= indices) || any(indices >= limit)
        error('BinSparse:InvalidMatrix', ...
              '%s contains an invalid index', name);
    end
end
end

function pointers = checked_pointers(pointers, expected, count)
if ~isnumeric(pointers) || ~isreal(pointers)
    error('BinSparse:InvalidMatrix', ...
          'pointers_to_1 must be real numeric data');
end
pointers = double(pointers(:));
if numel(pointers) ~= expected || isempty(pointers) || ...
        pointers(1) ~= 0 || pointers(end) ~= count || ...
        any(~isfinite(pointers)) || any(fix(pointers) ~= pointers) || ...
        any(diff(pointers) < 0)
    error('BinSparse:InvalidMatrix', 'Invalid pointers_to_1 array');
end
end

function expanded = expand_major_indices(major, counts)
if numel(major) ~= numel(counts)
    error('BinSparse:InvalidMatrix', 'Major index and pointer size mismatch');
end
expanded = repelem(double(major(:)), double(counts(:)));
expanded = expanded(:);
end

function require_ordered_pairs(first, second, format)
% Scanned in blocks, so the check costs a fixed few megabytes rather than
% several nnz-length temporaries.  Consecutive blocks overlap by one entry so
% that no adjacent pair straddles a block boundary unchecked.
count = numel(first);
if count < 2
    return;
end
lo = 1;
while lo < count
    hi = min(count, lo + scan_block_size());
    step = diff(first(lo:hi));
    bad = step < 0;
    tied = (step == 0);
    clear step
    if any(tied)
        bad = bad | (tied & diff(second(lo:hi)) <= 0);
    end
    if any(bad)
        error('BinSparse:InvalidMatrix', ...
              '%s indices are not sorted and unique', format);
    end
    lo = hi;
end
end

function n = scan_block_size()
n = 4194304;
end

function require_segment_order(indices, pointers, format)
for k = 1:numel(pointers)-1
    first = pointers(k) + 1;
    last = pointers(k + 1);
    if last > first && any(diff(indices(first:last)) <= 0)
        error('BinSparse:InvalidMatrix', ...
              '%s minor indices are not sorted and unique', format);
    end
end
end

function require_nonempty_segments(pointers, format)
if any(diff(pointers) <= 0)
    error('BinSparse:InvalidMatrix', ...
          '%s must not store empty major dimensions', format);
end
end

function require_strictly_increasing(values, label)
% Blocked for the same reason as require_ordered_pairs: a CVEC index array is
% nnz long, so diff() over the whole of it is not affordable at scale.
count = numel(values);
if count < 2
    return;
end
lo = 1;
while lo < count
    hi = min(count, lo + scan_block_size());
    if any(diff(values(lo:hi)) <= 0)
        error('BinSparse:InvalidMatrix', ...
              '%s are not sorted and unique', label);
    end
    lo = hi;
end
end

function value = text_scalar(value, name)
if isstring(value) && isscalar(value)
    value = char(value);
elseif iscell(value) && isscalar(value)
    value = char(value{1});
elseif ischar(value) && (isrow(value) || isempty(value))
    value = char(value);
else
    error('BinSparse:InvalidMetadata', ...
          'Metadata field %s must be scalar text', name);
end
end

function value = text_rows(value)
% Rebuild a Matlab char matrix from metadata text.  The text is normally a
% single multi-line string, but older files store an array of rows, so any
% embedded line terminators are split out of every element.  Rows are never
% deblanked here: whatever padding the writer kept is what char() needs to
% restore the original width.
if ischar(value)
    value = num2cell(value, 2);
elseif isstring(value)
    value = cellstr(value(:));
elseif iscellstr(value)
    value = value(:);
else
    error('BinSparse:InvalidMetadata', 'notes must contain text');
end
rows = cell(0, 1);
for k = 1:numel(value)
    pieces = regexp(value{k}, '\r\n|\n|\r', 'split');
    rows = [rows; pieces(:)];                                   %#ok<AGROW>
end
value = char(rows);
end

function value = normalize_component_text(value)
% The MATLAB class of a text component is carried by the HDF5 string datatype
% and restored by binsparse_read_string_dataset, so it is preserved here rather
% than inferred.  A char matrix stays a char matrix and a cellstr stays a
% cellstr; only a string array, which a foreign reader may produce, is mapped
% onto one of the two classes SSMC uses.
if ischar(value)
    return;
elseif iscellstr(value)
    value = value(:);
    for k = 1:numel(value)
        value{k} = reshape(char(value{k}), 1, []);
    end
elseif isstring(value)
    if isscalar(value)
        value = char(value);
    else
        value = cellstr(value(:));
    end
else
    error('BinSparse:InvalidComponent', 'Text component is invalid');
end
end

function ok = is_text(value)
ok = ischar(value) || iscellstr(value) || isstring(value);
end

function ok = starts_with(value, prefix)
ok = strncmp(value, prefix, numel(prefix));
end

function ok = ends_with(value, suffix)
ok = numel(value) >= numel(suffix) && ...
     strcmp(value(end-numel(suffix)+1:end), suffix);
end
