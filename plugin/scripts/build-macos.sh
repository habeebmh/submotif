#!/bin/bash
# Compatibility entry point: retains the original arm64 output used by the held
# release packager. For Intel/universal builds use build.py --target ... .
set -euo pipefail
project_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
case "${1:-}" in
    '') build_dir="$project_dir/build"; extra=() ;;
    --core-only) build_dir="$project_dir/build-core"; extra=(--core-only) ;;
    *) echo "Usage: $0 [--core-only]. See build.py --help for all platforms." >&2; exit 2 ;;
esac
if [[ $# -gt 1 ]]; then echo "Usage: $0 [--core-only]" >&2; exit 2; fi
exec python3 "$project_dir/scripts/build.py" --target macos-arm64 \
    --build-dir "$build_dir" --jobs "${UNDERTOW_JOBS:-4}" "${extra[@]}"
