#!/bin/bash

wsdir="$(dirname $(realpath $0))"

if [[ -z $(which aclocal) \
     || -z $(which autoheader) \
     || -z $(which autoconf) \
     || -z $(which automake) ]]; then
    echo "ERROR: aclocal, autoheader, autoconf and automake are required."
    exit 1
fi

cd "$wsdir"

aclocal
autoheader
autoconf

./Makefile.sh
automake

mkdir -p build
cd build
# "$wsdir/configure" --srcdir="$wsdir" --prefix="$(pwd)" "$@"
"$wsdir/configure" --srcdir="$wsdir" --prefix="$(pwd)" --with-test "$@"
# "$wsdir/configure" --srcdir="$wsdir" --prefix="$(pwd)" --with-debug "$@"
# "$wsdir/configure" --srcdir="$wsdir" --prefix="$(pwd)" --with-test --with-debug "$@"
