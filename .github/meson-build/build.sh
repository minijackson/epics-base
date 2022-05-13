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

function run() {
	echo "$@"
	"$@"
}

run meson --version
run meson --help
run meson setup --help

run meson "${crossArgs[@]}" build "${source_path}"
run ninja -C build
run meson test -C build
run sudo meson install -C build
run rm -rf build
