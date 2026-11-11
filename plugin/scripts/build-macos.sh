#!/bin/bash
set -euo pipefail

project_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
# JUCE's nested build helpers must find the same CMake/Ninja executables.
repo_dir="$(cd "$project_dir/.." && pwd)"
export PATH="$repo_dir/.tools/venv/bin:$PATH"
cmake_bin="$(command -v cmake || true)"
ctest_bin="$(command -v ctest || true)"
if [[ -z "$cmake_bin" || -z "$ctest_bin" ]]; then
    echo "CMake 3.22 or later is required. Install it or provide .tools/venv/bin/cmake." >&2
    exit 1
fi

core_only=OFF
case "${1:-}" in
    '') build_dir="$project_dir/build" ;;
    --core-only) core_only=ON; build_dir="$project_dir/build-core" ;;
    *) echo "Usage: $0 [--core-only]" >&2; exit 2 ;;
esac
if [[ $# -gt 1 ]]; then echo "Usage: $0 [--core-only]" >&2; exit 2; fi
generator="Unix Makefiles"
if command -v ninja >/dev/null 2>&1; then generator=Ninja; fi

"$cmake_bin" -S "$project_dir" -B "$build_dir" -G "$generator" \
    -DCMAKE_BUILD_TYPE=Release -DCMAKE_OSX_ARCHITECTURES=arm64 \
    -DCMAKE_OSX_DEPLOYMENT_TARGET=11.0 -DUNDERTOW_CORE_ONLY="$core_only"
"$cmake_bin" --build "$build_dir" --config Release --parallel "${UNDERTOW_JOBS:-4}"
# The VST3 manifest helper writes resources after linking. Sign the completed
# development bundles, so that those resources are included in the seal.
if [[ "$core_only" == OFF ]]; then
    for bundle in "$build_dir/Undertow_artefacts/Release/VST3/Submotif.vst3" \
                  "$build_dir/Undertow_artefacts/Release/Standalone/Submotif.app"; do
        codesign --force --deep --sign - "$bundle"
        codesign --verify --deep --strict "$bundle"
    done
fi
"$ctest_bin" --test-dir "$build_dir" -C Release --output-on-failure

if [[ "$core_only" == OFF ]]; then
    if ! command -v node >/dev/null 2>&1; then
        echo "C++ checks passed. Node.js is required to run plugin/tests/max-tests.js." >&2
        exit 1
    fi
    node "$project_dir/tests/max-tests.js"
    if ! command -v python3 >/dev/null 2>&1; then
        echo "Python 3 is required to compare VST3 and Max generation." >&2
        exit 1
    fi
    python3 "$project_dir/tests/generation-parity.py"
    echo "VST3: $build_dir/Undertow_artefacts/Release/VST3/Submotif.vst3"
fi
