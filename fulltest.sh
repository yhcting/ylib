#!/bin/bash

set -e

topdir=$(readlink -e $(dirname $0))

pushd $topdir >/dev/null

function usage() {
    prog=$1
    cat <<EOF
Run test for ylib.

$prog [OPTION]

OPTION
    -d
        Skip doxygen test.
    -u
        Skip unit test.
EOF
}


if [[ $(git diff | wc -l) != 0 ]]; then
    echo "ERROR: There are files that are NOT cached or commited! Abort!"
    exit 1
fi


while getopts "hdu" opt; do
    case $opt in
        h)
            usage
            exit 0
            ;;
        d)
            skipDoxy=true
            ;;
        u)
            skipUnit=true
            ;;
        \?)
            echo "Invalid option: -$OPTARG" >&2
            exit 1
            ;;
        :)
            echo "Option -$OPTARG requires an argument." >&2
            exit 1
            ;;
    esac
done

shift $((OPTIND-1))


# Test doxygen
# ------------
doxyerr=$(doxygen Doxyfile 2>&1 1>/dev/null | wc -l)
if [ 0 != $doxyerr ]; then
    echo "Doxygen has warnings or errors" >&2
    exit 1
fi

# Test build and unit
# -------------------
nrcores=$(cat /proc/cpuinfo | grep processor | wc -l)
cfgopts="--with-debug"
for opt in "" $cfgopts; do
    git clean -dfx
    ./configure-full --prefix="$topdir" $opt
    make -j$nrcores install
    if [ x$skipUnit != xtrue -a -e bin/y ]; then
        bin/y
    fi
done

echo ">> Congraturations !"

popd >/dev/null
