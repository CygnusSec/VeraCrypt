#!/usr/bin/env bash
# Copyright (c) 2013-2025 AM Crypto
# Governed by the Apache License 2.0 the full text of which is contained
# in the file License.txt included in VeraCrypt binary and source
# code distribution packages.
#

# Absolute path to this script
SCRIPT=$(readlink -f "$0")
# Absolute path this script is in
SCRIPTPATH=$(dirname "$SCRIPT")
# source directory which contains the Makefile
SOURCEPATH=$(readlink -f "$SCRIPTPATH/..")
# directory where the VeraCrypt has been checked out
PARENTDIR=$(readlink -f "$SCRIPTPATH/../../..")

external_module_sources=()
external_module_build_dir=""

usage() {
    echo "Usage: $0 [-m module-source-directory]..."
    echo "  -m <dir>  Build and embed an external encryption module source tree"
    echo "            (repeat -m to include multiple modules)"
}

while getopts "m:h" flag; do
    case "$flag" in
        m) external_module_sources+=("$OPTARG") ;;
        h) usage; exit 0 ;;
        *) usage >&2; exit 1 ;;
    esac
done
shift $((OPTIND - 1))
if [ "$#" -ne 0 ]; then
    usage >&2
    exit 1
fi

# Make sure only root can run our script
if [ "$(id -u)" != "0" ]; then
   echo "VeraCrypt must be built by root" 1>&2
   exit 1
fi

# Check the condition of wxBuildConsole and wxWidgets-3.2.5 in the original PARENTDIR
if [ -d "$PARENTDIR/wxBuildConsole" ]; then
    echo "Using existing PARENTDIR: $PARENTDIR, wxBuildConsole is present."
elif [ -d "$PARENTDIR/wxWidgets-3.2.5" ]; then
    echo "Using existing PARENTDIR: $PARENTDIR, wxWidgets-3.2.5 is present."
else
    # Change PARENTDIR to /tmp and check conditions again
    export PARENTDIR="/tmp"
    if [ -d "$PARENTDIR/wxBuildConsole" ]; then
        echo "Switched to PARENTDIR: /tmp, wxBuildConsole is present in /tmp."
    elif [ -d "$PARENTDIR/wxWidgets-3.2.5" ]; then
        echo "Switched to PARENTDIR: /tmp, wxWidgets-3.2.5 is present in /tmp."
    else
        echo "Error: Neither wxBuildConsole nor wxWidgets-3.2.5 found in /tmp. Exiting."
        exit 1
    fi
fi

# The sources of wxWidgets 3.2.5 must be extracted to the parent directory
export WX_ROOT=$PARENTDIR/wxWidgets-3.2.5

if [ "${#external_module_sources[@]}" -gt 0 ]; then
    for external_module_source in "${external_module_sources[@]}"; do
        if [ ! -d "$external_module_source/src" ]; then
            echo "Error: External module source directory does not exist: $external_module_source/src" >&2
            exit 1
        fi
    done
    external_module_build_dir=$(mktemp -d "${TMPDIR:-/tmp}/veracrypt-modules.XXXXXX")
    trap 'rm -rf "$external_module_build_dir"' EXIT
    "$SCRIPTPATH/build_external_encryption_modules.sh" \
        "$external_module_build_dir" "${external_module_sources[@]}"
    export VC_EXTERNAL_MODULE_DIR=$external_module_build_dir
    echo "Embedding built external encryption modules from $VC_EXTERNAL_MODULE_DIR"
fi

cd $SOURCEPATH

echo "Building GUI version of VeraCrypt"

# this will be the temporary wxWidgets directory
export WX_BUILD_DIR=$PARENTDIR/wxBuildGui

# Check if wx-config exists in WX_BUILD_DIR
if [ -L "${WX_BUILD_DIR}/wx-config" ]; then
    echo "wx-config already exists in ${WX_BUILD_DIR}. Skipping wxbuild."
else
    echo "Using wxWidgets sources in $WX_ROOT"
    make WXSTATIC=1 wxbuild || exit 1
fi
make WXSTATIC=1 clean || exit 1
make WXSTATIC=1 || exit 1
make WXSTATIC=1 package || exit 1
make WXSTATIC=1 appimage || exit 1

echo "Building console version of VeraCrypt"

# this will be the temporary wxWidgets directory
export WX_BUILD_DIR=$PARENTDIR/wxBuildConsole

# Check if wx-config exists in WX_BUILD_DIR
if [ -L "${WX_BUILD_DIR}/wx-config" ]; then
    echo "wx-config already exists in ${WX_BUILD_DIR}. Skipping wxbuild."
else
    echo "Using wxWidgets sources in $WX_ROOT"
    make WXSTATIC=1 NOGUI=1 wxbuild || exit 1
fi
make WXSTATIC=1 NOGUI=1 clean || exit 1
make WXSTATIC=1 NOGUI=1 || exit 1
make WXSTATIC=1 NOGUI=1 package || exit 1
