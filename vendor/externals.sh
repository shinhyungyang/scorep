#!/bin/sh

set -e

bindir=${0%/*}
: ${bindir:=.}
cd "$bindir"

cmd_get()
{
    local external=$1
    shift
    test ! -d "$external" && {
        echo "git clone $* $bindir/$external"
        eval 'git '"${EXTERNALS_GIT_OPTIONS-}"' clone --quiet "$@" "$external"'
        if [ -x "$external"/vendor/externals.sh ]; then
            "$external"/vendor/externals.sh get
        fi
    }
}

cmd_update()
{
    local external=$1
    test -d "$external" && {
        echo "cd $bindir/$external && git pull"
        eval '( cd "$external" && git '"${EXTERNALS_GIT_OPTIONS-}"' pull --quiet )'
        if [ -x "$external"/vendor/externals.sh ]; then
            "$external"/vendor/externals.sh update
        fi
    }
}

cmd_clean()
{
    local external=$1
    echo "rm -rf $bindir/$external"
    rm -rf "$external"
}

cmd_help()
{
    printf "Usage: $0 [get|update|clean]\n"
    exit 1
}

cmd=cmd_${1:-get}

${cmd} otf2 -b master https://gitlab.jsc.fz-juelich.de/perftools/otf2.git
${cmd} opari2 -b v2.0.8 https://gitlab.jsc.fz-juelich.de/perftools/opari2.git
${cmd} cubew -b master https://gitlab.jsc.fz-juelich.de/perftools/cubew.git
${cmd} cubelib -b master https://gitlab.jsc.fz-juelich.de/perftools/cubelib.git
