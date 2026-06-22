#!/usr/bin/env bash
# Copyright 2024 RnD Center "ELVEES", JSC

set -eEu

config_path=$(realpath $1)
config_basename=$(basename $config_path)

function error {
    >&2 echo Error: "$*"
    exit 1
}

[[ -f $config_path ]] || error $config_path doesn\'t exist

pipenv install --skip-lock

if ! grep -q CONFIG_PLATFORM $config_path; then
    export CROSS_COMPILE=mipsel-elvees-elf-
    export PATH=/usr/corpneo/toolchains/mcom03-buildroot/latest/linux510/opt/toolchain-mipsel-elvees-elf32/bin:$PATH
fi

[[ ! -z "${CROSS_COMPILE}" ]] || error Unsupported platform in defconfig

set +x
pipenv run make clean-all
pipenv run make $config_basename
pipenv run make -j $(nproc) CFLAGS="-Wall -Werror"

echo Checking stack
pipenv run make check-stack
