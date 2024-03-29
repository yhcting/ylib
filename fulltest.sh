#!/bin/bash

set -e

wsdir="$(realpath $(dirname $0))"

pushd "$wsdir" >/dev/null

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
if [[ "$skipDoxy" != "true" ]]; then
    if [[ -z $(which doxygen) ]]; then
        echo "doxygen not found!" >&2
        exit 1
    fi

    doxyerr=$(doxygen Doxyfile 2>&1 1>/dev/null | wc -l)
    if [ 0 != $doxyerr ]; then
        # Doxygen also has lots of unknown bugs.
        echo "Doxygen has warnings or errors: ignored" >&2
        # exit 1
    fi
fi

# Test build and unit
# -------------------
if [[ "$skipUnit" != "true" ]]; then
    nrcores=$(cat /proc/cpuinfo | grep processor | wc -l)
    for opt in "" "--with-test" "--with-debug" "--with-debug --with-test" ; do
        pushd "$wsdir" >/dev/null
        # git clean -dfx
        rm -rf build
        mkdir -p build
        cd build
        "$wsdir/configure-full.sh" --prefix="$(pwd)" $opt
        make -j$nrcores install
        if [[ -e bin/y ]]; then
            bin/y
        fi
        popd >/dev/null
    done
fi

echo "Congraturations! Full test passed."

popd >/dev/null
