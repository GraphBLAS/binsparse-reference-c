% binsparse
%
% Files
%   binsparse_read                             - read a sparse matrix from a binsparse hd5 file
%   binsparse_write                            - write a matrix to a file in binsparse hd5 format
%   binsparse_from_ssmc                        - convert SSMC A+Zeros to a Binsparse matrix struct
%   binsparse_minimize_types                   - minimize value/index types in a Binsparse struct
%   generate_bsp_from_ssmc                     - write SSMC problem to a Binsparse file
%
%   bsp_matrix_create                          - SPDX-FileCopyrightText: 2024 Binsparse Developers
%   bsp_matrix_info                            - SPDX-FileCopyrightText: 2024 Binsparse Developers
%
%   build_matlab_bindings                      - SPDX-FileCopyrightText: 2024 Binsparse Developers
%   build_octave_bindings                      - SPDX-FileCopyrightText: 2024 Binsparse Developers
%   compile_binsparse_read                     - SPDX-FileCopyrightText: 2024 Binsparse Developers
%   compile_binsparse_read_octave              - SPDX-FileCopyrightText: 2024 Binsparse Developers
%   compile_binsparse_write                    - SPDX-FileCopyrightText: 2024 Binsparse Developers
%   compile_binsparse_write_octave             - SPDX-FileCopyrightText: 2024 Binsparse Developers
%   compile_write_binsparse_from_matlab        - SPDX-FileCopyrightText: 2024 Binsparse Developers
%   compile_write_binsparse_from_matlab_octave - SPDX-FileCopyrightText: 2024 Binsparse Developers
%   test_binsparse_read                        - SPDX-FileCopyrightText: 2024 Binsparse Developers
%   test_binsparse_write                       - SPDX-FileCopyrightText: 2024 Binsparse Developers
%   test_bsp_matrix_struct                     - SPDX-FileCopyrightText: 2024 Binsparse Developers
%   test_binsparse_from_ssmc                   - SPDX-FileCopyrightText: 2024 Binsparse Developers
%   test_binsparse_minimize_roundtrip          - SPDX-FileCopyrightText: 2024 Binsparse Developers
%   test_generate_bsp_from_ssmc                - SPDX-FileCopyrightText: 2024 Binsparse Developers
%   test_write_binsparse_from_matlab           - SPDX-FileCopyrightText: 2024 Binsparse Developers
