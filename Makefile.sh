#!/bin/bash

#
# This is source of Makefile.am
#

modules="
:crc
:dynb
:graph
:hashl
:hash
:heap
:listl
:list
:lru
:mempool
:msg
:msgq
:msglooper
:msghandler
:threadex
:task
:taskmanager
:taskdepman
:p
:o
:gp
:pool
:set
:statmath
:statprint
:treel
:trie
:log
:errno
:ut
geo:point
geo:line
geo:band
geo:rect
geo:rgn
linux:proc
"

##############################################################################
#
# Utility functions
#
##############################################################################
#
# $1 (out) component name
# $2 (out) module name
# $3 (in) full module name
#
function get_comp_mod() {
    comp=$1
    mod=$2
    fullname=$3
    eval "$comp=$(echo $fullname | cut -d':' -f1)"
    eval "$mod=$(echo $fullname | cut -d':' -f2)"
}

# $1 full module name
function header_file() {
    get_comp_mod comp mod $1
    if [[ -z $comp ]]; then
        echo y${mod}.h
    else
        echo ${comp}/y${mod}.h
    fi
}

# $1 full module name
function src_file() {
    get_comp_mod comp mod $1
    if [[ -z $comp ]]; then
        echo ${mod}.c
    else
        echo ${comp}/${mod}.c
    fi
}

##############################################################################
#
# Create Makefile.am
#
##############################################################################
# Headers
# -------
common_headers="
ydef.h
ylib.h
"

inc_headers=
for h in $common_headers; do
    inc_headers+=" $h"
done
for m in $modules; do
    hdrf=$(header_file $m)
    inc_headers+=" $hdrf"
done


# Library
# -------
common_sources="
lib.c
"

lib_sources=
for s in $common_sources; do
    lib_sources+=" $s"
done
for m in $modules; do
    srcf=$(src_file $m)
    lib_sources+=" $srcf"
done



# Test program
# ------------
common_sources="
main.c
"
prog_sources=
for s in $common_sources; do
    prog_sources+=" $s"
done
for m in $modules; do
    srcf=$(src_file $m)
    prog_sources+=" $srcf"
done


# Create Makefile.am
# ------------------
cat <<EOF >Makefile.am
##############################################################################
#
# This is auto-generated by Makefile.sh
# DO NOT EDIT this file directly
#
##############################################################################

SUBDIRS = src tests
EOF

cat <<EOF >src/Makefile.am
##############################################################################
#
# This is auto-generated by Makefile.sh
# DO NOT EDIT this file directly
#
##############################################################################

nobase_include_HEADERS = $inc_headers

pkglib_LTLIBRARIES = liby.la
liby_la_SOURCES = $lib_sources
EOF

cat <<EOF >tests/Makefile.am
##############################################################################
#
# This is auto-generated by Makefile.sh
# DO NOT EDIT this file directly
#
##############################################################################

if DEBUG
    bin_PROGRAMS = y
    y_SOURCES = $prog_sources
    y_CFLAGS = -I../src

    y_LDADD = ../src/liby.la -lpthread -lrt -lm
endif

EOF


##############################################################################
#
# Create Android.mk
#
##############################################################################
autogenfile="Android.mk"

cat <<EOF >$autogenfile
##############################################################################
#
# This is auto-generated by Makefile.sh
# DO NOT EDIT this file directly
#
##############################################################################

LOCAL_PATH := \$(call my-dir)

include \$(CLEAR_VARS)

LOCAL_MODULE := y

LOCAL_SRC_FILES := \\
EOF

for m in $modules; do
    srcf=$(src_file $m)
    echo "	$srcf \\" >>$autogenfile
done

cat <<EOF >>$autogenfile


#LOCAL_FORCE_STATIC_EXECUTABLE := true
LOCAL_CFLAGS += -Wall -Werror -DCONFIG_IGNORE_CONFIG
LOCAL_C_INCLUDES +=

include \$(BUILD_SHARED_LIBRARY)
EOF
