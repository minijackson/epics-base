#!/usr/bin/env bash

set -euo pipefail
IFS=$'\n\t'

source_path="$1"

typeset -a crossArgs=()

if [[ "${CROSS:-false}" == true ]]; then
	cat > cross.ini <<EOF
[properties]
needs_exe_wrapper = true

[host_machine]
system = '${HOST_SYSTEM}'
cpu_family = '${HOST_CPU_FAMILY}'
cpu = '${HOST_CPU}'
endian = '${HOST_ENDIAN}'

[binaries]
c = '${HOST_COMPILER_PREFIX}gcc'
cpp = '${HOST_COMPILER_PREFIX}g++'
strip = '${HOST_COMPILER_PREFIX}strip'
pkgconfig = '${HOST_COMPILER_PREFIX}pkg-config'
EOF

	echo "::group::Cross file"
	cat cross.ini

	crossArgs=("--cross-file" "cross.ini")
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
