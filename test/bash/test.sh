#!/usr/bin/env bash

# SPDX-FileCopyrightText: 2024 Binsparse Developers
#
# SPDX-License-Identifier: BSD-3-Clause

set -e

convert_ssmc() {
  echo "unpacking $1"
  fpath=$1

  format=$2

  if [ -z "$format" ]
  then
    format=COO
  else
    format=$2
  fi

  format_string=`echo ${format} | tr '[:upper:]' '[:lower:]'`

  echo "Writing in format ${format}."

  fname=`basename ${fpath}`
  directory=`dirname ${fpath}`
  tar -xf $fpath -C $directory

  matrix_name=${fname/.tar.gz/}

  main_matrix=$directory/$matrix_name/$matrix_name.mtx

  dest_file=$directory/$matrix_name.${format_string}.bsp.h5

  # echo "dest file ${dest_file}"

  # echo "Primary matrix is \"${main_matrix}\" -> ${dest_file}/root"
  echo "mtx2bsp ${main_matrix} ${dest_file} ${format}"
  mtx2bsp ${main_matrix} ${dest_file} ${format}

  # Set "null option" to return an empty list if no files match glob.
  shopt -s nullglob

  for secondary_matrix in ${directory}/${matrix_name}/${matrix_name}_*.mtx
  do
    # echo "Secondary matrix \"${secondary_matrix}\""
    secondary_name=`basename ${secondary_matrix}`
    secondary_name=${secondary_name/.mtx/}
    secondary_name=${secondary_name/${matrix_name}_/}
    echo "mtx2bsp ${secondary_matrix} ${dest_file}:${secondary_name}"
    mtx2bsp ${secondary_matrix} ${dest_file}:${secondary_name}
  done

  rm -r ${directory}/${matrix_name}
}

check_ssmc() {
  echo "unpacking $1"
  fpath=$1

  format=$2

  if [ -z "$format" ]
  then
    format=COO
  else
    format=$2
  fi

  format_string=`echo ${format} | tr '[:upper:]' '[:lower:]'`

  fname=`basename ${fpath}`
  directory=`dirname ${fpath}`
  tar -xf $fpath -C $directory

  matrix_name=${fname/.tar.gz/}

  main_matrix=$directory/$matrix_name/$matrix_name.mtx

  dest_file=$directory/$matrix_name.${format_string}.bsp.h5

  # echo "dest file ${dest_file}"

  # echo "Primary matrix is \"${main_matrix}\" -> ${dest_file}/root"
  echo "check_equivalence ${main_matrix} ${dest_file}"
  check_equivalence ${main_matrix} ${dest_file}

  # Set "null option" to return an empty list if no files match glob.
  shopt -s nullglob

  for secondary_matrix in ${directory}/${matrix_name}/${matrix_name}_*.mtx
  do
    # echo "Secondary matrix \"${secondary_matrix}\""
    secondary_name=`basename ${secondary_matrix}`
    secondary_name=${secondary_name/.mtx/}
    secondary_name=${secondary_name/${matrix_name}_/}
    echo "check_equivalence ${secondary_matrix} ${dest_file}:${secondary_name}"
    check_equivalence ${secondary_matrix} ${dest_file}:${secondary_name}
  done

  rm -r ${directory}/${matrix_name}
}

export PATH=$PATH:$PWD/../../examples

convert_ssmc $1 COO
check_ssmc $1 COO
