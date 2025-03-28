#!/usr/bin/env bash
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

# Set the stage for the tutorial

SUBDIRECTORY_OK=Yes
USAGE="\
begin [number]
or: wrapgen_tutorial.sh end
--
begin  Start the tutorial for mpi_wrapgen2
end Exit the tutorial"
. $(git --exec-path)/git-sh-setup
require_work_tree

IN_TUTORIAL_MARKER='.IN_TUTORIAL'
PYTHON=python3

set -eu

current_tutorial()
{
    if [ -f "$IN_TUTORIAL_MARKER" ]; then
        cat "$IN_TUTORIAL_MARKER"
    else
        echo "0"
    fi
}

arrange_tutorial()
{
    case "$1" in
    (1|2|3|4|5)
        patch_file="doc/Tutorials/patches/$1"
        if [ -f "$patch_file" ]; then
            git apply "doc/Tutorials/patches/$1"
        fi
        echo "$1" > "$IN_TUTORIAL_MARKER"
        ;;
    (*)
        die "Tutorial $1 not known"
        ;;
    esac
}

begin_tutorial()
{
    if [ "$(current_tutorial)" != "0" ] ;  then
        die "You are already in a tutorial. End the tutorial with 'wrapgen-tutorial.sh end'"
    fi
    require_clean_work_tree "begin tutorial" ""
    arrange_tutorial "$1"
    say "Welcome to the mpi_wrapgen2 tutorial, part $1!"
}

end_tutorial()
{
    if ! $IN_TUTORIAL; then
        die "Not in a tutorial. Nothing is done."
    fi
    if nuke_working_tree; then
        say "Working tree is reset."
        rm -f "$IN_TUTORIAL_MARKER"
        say "You have ended this tutorial session. Bye!"
    else
        say "Cancelled the reset. You are still in the tutorial."
    fi
}

nuke_working_tree()
{
    say "This will hard-reset your git working tree. Do you want to proceed? [No/Yes]"
    local sure
    read sure
    case $sure in
    (YES|Yes|yes)
        git reset --hard HEAD && git clean -fd
        ;;
    (*)
        return 1
        ;;
    esac
}

if [ ! -f 'mpi_wrapgen2.sh' ]; then
    die "This script has to be called from the mpi_wrapgen2 directory"
fi

if [ $# -lt 1 ]; then
    usage
fi

IN_TUTORIAL=false
if [ -f "$IN_TUTORIAL_MARKER" ]; then
    IN_TUTORIAL=true
fi

TUTORIAL_NUMBER=1
case "$1" in
(begin)
    shift
    if [ $# -ge 1 ]; then
        TUTORIAL_NUMBER="$1"
        case "$TUTORIAL_NUMBER" in
        (1|Generating*) TUTORIAL_NUMBER=1 ;;
        (2|Examining*) TUTORIAL_NUMBER=2 ;;
        (3|Adding*) TUTORIAL_NUMBER=3 ;;
        (4|Customizing*) TUTORIAL_NUMBER=4 ;;
        (5|Writing*) TUTORIAL_NUMBER=5 ;;
        (*) usage ;;
        esac
    fi
    begin_tutorial "$TUTORIAL_NUMBER"
    ;;
(end)
    end_tutorial
    ;;
(*)
    usage
    ;;
esac
