#!/usr/bin/env bash

set -euo pipefail
IFS=$'\n\t'

source_path="$1"

typeset -a crossArgs=()

if [[ "${CROSS:-false}" == true ]]; then
	echo > cross.ini <<EOF
[properties]
needs_exe_wrapper = true

[host_machine]
system = '${HOST_SYSTEM}'
cpu_family = '${HOST_CPU_FAMILY}'
cpu = '${HOST_CPU}'
endian = '${HOST_ENDIAN}'
EOF

	crossArgs=("--cross-file" "cross.ini")

	export CC_FOR_BUILD="gcc"
	export CXX_FOR_BUILD="g++"
	export CC="${HOST_COMPILER_PREFIX}gcc"
	export CXX="${HOST_COMPILER_PREFIX}g++"
fi

function run() {
	echo "::group::" "$@"
	"$@"
}

run meson --version
run ninja --version

run meson "${crossArgs[@]}" build "${source_path}"
run ninja -C build
run meson test -C build
run sudo meson install -C build
run rm -rf build
