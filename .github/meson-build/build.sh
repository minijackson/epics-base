#!/usr/bin/env bash

set -euo pipefail
IFS=$'\n\t'

source_path="$1"

typeset -a crossArgs=()

if [[ "${CROSS:-false}" == true ]]; then
	echo > cross.ini <<EOF
[host_machine]
system = '${HOST_SYSTEM}'
cpu_family = '${HOST_CPU_FAMILY}'
cpu = '${HOST_CPU}'
endian = '${HOST_ENDIAN}'
EOF

	crossArgs=("--cross-file" "cross.ini")

	export CC="${HOST_COMPILER_PREFIX}gcc"
	export CXX="${HOST_COMPILER_PREFIX}g++"
fi

meson "${crossArgs[@]}" setup build "${source_path}"
ninja -C build
meson test -C build
sudo meson install -C build
rm -rf build
