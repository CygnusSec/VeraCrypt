#!/usr/bin/env bash
set -euo pipefail

if [ "$#" -lt 2 ]; then
  echo "Usage: $0 <output-directory> <module-source-directory> [...]" >&2
  exit 1
fi
SCRIPT_DIR=$(cd "$(dirname "$0")" && pwd)
VERACRYPT_DIR=$(cd "$SCRIPT_DIR/../.." && pwd)
OUTPUT_DIR=$1
shift
EXTERNAL_MODULE_CFLAGS=()
if [ -n "${VC_EXTERNAL_MODULE_CFLAGS:-}" ]; then
  read -r -a EXTERNAL_MODULE_CFLAGS <<< "$VC_EXTERNAL_MODULE_CFLAGS"
fi
mkdir -p "$OUTPUT_DIR"
case "$(uname -s)" in
  Darwin) MODULE_EXTENSION=dylib; SHARED_FLAGS=(-dynamiclib) ;;
  Linux|FreeBSD) MODULE_EXTENSION=so; SHARED_FLAGS=(-shared -fPIC) ;;
  *) echo "Unsupported external-module build host: $(uname -s)" >&2; exit 1 ;;
esac
for module_dir in "$@"; do
  module_dir=$(cd "$module_dir" && pwd)
  source_dir="$module_dir/src"
  module_name=$(printf "%s" "$(basename "$module_dir")" | tr -c "A-Za-z0-9._-" "-")
  output="$OUTPUT_DIR/libveracrypt-${module_name}.${MODULE_EXTENSION}"
  if [ ! -f "$source_dir/veracrypt_cipher_adapter.c" ]; then
    echo "Error: $source_dir/veracrypt_cipher_adapter.c is required" >&2; exit 1
  fi
  sources=()
  while IFS= read -r -d "" source; do sources+=("$source"); done < <(find "$source_dir" -maxdepth 1 -type f \( -name "*.c" -o -name "*.cc" -o -name "*.cpp" \) -print0 | sort -z)
  if [ "${#sources[@]}" -eq 0 ]; then echo "Error: no sources found in $source_dir" >&2; exit 1; fi
  compiler=${CC:-cc}
  for source in "${sources[@]}"; do case "$source" in *.cc|*.cpp) compiler=${CXX:-c++}; break ;; esac; done
  echo "Building external encryption module: $module_dir"
  if [ "${#EXTERNAL_MODULE_CFLAGS[@]}" -gt 0 ]; then
    "$compiler" "${SHARED_FLAGS[@]}" "${EXTERNAL_MODULE_CFLAGS[@]}" -O2 -fvisibility=hidden -I"$VERACRYPT_DIR/src/Volume" -I"$source_dir" "$VERACRYPT_DIR/src/Volume/GenericEncryptionModule.c" "${sources[@]}" -o "$output"
  else
    "$compiler" "${SHARED_FLAGS[@]}" -O2 -fvisibility=hidden -I"$VERACRYPT_DIR/src/Volume" -I"$source_dir" "$VERACRYPT_DIR/src/Volume/GenericEncryptionModule.c" "${sources[@]}" -o "$output"
  fi
  echo "Built external encryption module: $output"
done
