#!/bin/bash
#
# This file is part of the Score-P software (http://www.score-p.org)
#
# Copyright (c) 2025,
# Forschungszentrum Juelich GmbH, Germany
#
# This software may be modified and distributed under the terms of
# a BSD-style license. See the COPYING file in the package base
# directory for details.
#
set -e

script_name=${0##*/}
# Enable calling this script from somewhere else
script_path=${0%/*}
: "${script_path:=.}"
cd "$script_path"

output_path=output
adapter_install_path=../../src/adapters/mpi
autoconf_install_path=../../build-config/m4

usage()
{
    echo "Usage: ${script_name} [--driver-args [arg...]]"
    exit 1
}

while [ $# -ne 0 ]; do
    case "$1" in
    (--driver-args)
        shift
        break;;
    (*)
        usage
        ;;
    esac
done
# Remaining args are passed directly to driver.py
driver_args=("$@")

print_warning()
{
     echo "WARNING: $1"
}

print_error()
{
    echo "ERROR: $1"
}

ensure_clean_output_directory()
{
    echo "Ensuring the output directory '${output_path}' is clean"
    mkdir -p $output_path
    rm -rf "${output_path:?}/"*
}

generate_outputs()
{
    echo "Generating outputs"
    ./driver.py "${driver_args[@]}"
}

count_warnings_and_todos()
{
    local todos; todos=$(grep -ri 'TODO:' $output_path | wc -l)
    local warnings; warnings=$(grep -ri 'WARNING:' $output_path | wc -l)
    print_warning "Found $todos todos and $warnings warnings."
}

remove_trailing_whitespace()
{
    sed -i 's/[[:space:]]\+$//' "$1"
}

beautify()
{
    echo "Beautifying outputs"
    local beautifier_config_path="../../common/beautifier"
    files_fprettify=$(find $output_path -type f -name "*.[Ff]90")
    for file in $files_fprettify; do
        echo "BEAUTIFY $file" \
        && fprettify -c "$beautifier_config_path/beautify_f.cfg" --line-length 1000 "$file" \
        && python3 f90_break_long_lines.py "$file" "$file.tmp" \
        && mv "$file.tmp" "$file" \
        && remove_trailing_whitespace "$file" &
    done

    files_uncrustify=$(find $output_path -type f -name "*.[h|c]")
    for file in $files_uncrustify; do
        echo "BEAUTIFY $file" \
        && uncrustify -c "$beautifier_config_path/beautify.cfg" --no-backup -L 1 "$file"
    done
    wait
}

get_install_path()
{
    local dir=$1
    case $dir in
    m4)
        echo "${autoconf_install_path}"
        return 0
        ;;
    adapter)
        echo "${adapter_install_path}"
        return 0
        ;;
    default)
        echo ""
        return 1
        ;;
    esac
}

generated_file_differs_from_installed()
{
    local generated=$1
    local installed=$2

    if diff -N "$generated" "$installed" > /dev/null; then
        return 1
    else
        diff_status=$?
        if [ $diff_status -eq 1 ]; then
            return 0
        fi
        print_error "Error in diff -N $generated $installed: diff exited with status $diff_status"
        exit 1
    fi
}

install_generated_files()
{
    echo "Installing generated files"
    for dir in adapter m4; do
        if [ ! -d "${output_path}/${dir}" ]; then
            continue
        fi
        local install_path
        install_path=$(get_install_path "$dir")

        to_be_installed=$(find $output_path/$dir -type f)
        for generated_file in $to_be_installed; do
            local installed_file; installed_file="$install_path/${generated_file#*/*/}"
            if generated_file_differs_from_installed "$generated_file" "$installed_file"; then
                echo "Updating file $installed_file"
                cp -f "$generated_file" "$installed_file"
            fi
        done
    done
}

ensure_clean_output_directory
generate_outputs
count_warnings_and_todos
beautify
install_generated_files
echo "Successfully generated and updated wrappers"
