#!/bin/bash

set -e

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
cfgopts="--with-debug --with-debug2"
for opt in "" $cfgopts; do
    make clean
    ./configure-full $opt
    make
    if [ x$skipUnit != xtrue ]; then
        ./y
    fi
done

echo ">> Congraturations !"
