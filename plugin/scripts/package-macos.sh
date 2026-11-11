#!/bin/bash
set -euo pipefail

project_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
repo_dir="$(cd "$project_dir/.." && pwd)"
plugin="$project_dir/build/Undertow_artefacts/Release/VST3/Submotif.vst3"
bundle_name=Submotif-macOS-arm64
dist_dir="$project_dir/dist"
if [[ ! -f "$plugin/Contents/MacOS/Submotif" ]]; then
    echo "Build the arm64 Release VST3 with plugin/scripts/build-macos.sh first." >&2
    exit 1
fi
for file in Submotif.amxd undertow.js undertow-core.js undertow-roll.js README.md; do
    if [[ ! -f "$project_dir/max/$file" ]]; then echo "Missing Max companion file: $file" >&2; exit 1; fi
done
for file in LICENSE README.md docs/CONTRIBUTING.md docs/THIRD_PARTY_NOTICES.md; do
    if [[ ! -f "$repo_dir/$file" ]]; then echo "Missing project document: $file" >&2; exit 1; fi
done
for file in liability.md license-guide.md terms.md privacy.md; do
    if [[ ! -f "$repo_dir/docs/legal/$file" ]]; then echo "Missing legal document: $file" >&2; exit 1; fi
done

# Ship the exact dependency source used by the pinned build, including all of
# its component notices. A dependency URL alone is not Corresponding Source.
juce_source="${UNDERTOW_JUCE_SOURCE:-$repo_dir/.deps/JUCE}"
if [[ ! -f "$juce_source/LICENSE.md" ]]; then juce_source="$project_dir/build/_deps/juce-src"; fi
juce_commit=dabd5ad13cd869cba41046947fffc8e5ce5cd662
if [[ ! -f "$juce_source/LICENSE.md" ]] || \
   [[ "$(git -C "$juce_source" rev-parse HEAD 2>/dev/null || true)" != "$juce_commit" ]]; then
    echo "Set UNDERTOW_JUCE_SOURCE to the pinned JUCE checkout used to build ($juce_commit)." >&2
    exit 1
fi
if [[ -n "$(git -C "$juce_source" status --porcelain --untracked-files=no)" ]]; then
    echo "JUCE has local changes. Review them and update source provenance before packaging." >&2
    exit 1
fi
if ! /usr/bin/lipo -verify_arch arm64 "$plugin/Contents/MacOS/Submotif"; then
    echo "The VST3 does not contain the required arm64 architecture." >&2
    exit 1
fi
mkdir -p "$dist_dir"
stage_dir="$(mktemp -d "$dist_dir/.submotif-package.XXXXXX")"
trap 'rm -rf "$stage_dir"' EXIT
staged_bundle="$stage_dir/$bundle_name"
mkdir -p "$staged_bundle/docs" "$staged_bundle/licenses"
/usr/bin/ditto "$plugin" "$staged_bundle/Submotif.vst3"
/usr/bin/ditto "$project_dir/max" "$staged_bundle/max"
cp "$repo_dir/LICENSE" "$repo_dir/README.md" "$staged_bundle/"
cp "$repo_dir/docs/CONTRIBUTING.md" "$repo_dir/docs/THIRD_PARTY_NOTICES.md" "$staged_bundle/docs/"
/usr/bin/ditto "$repo_dir/docs/legal" "$staged_bundle/docs/legal"
cp "$repo_dir/docs/MUSIC_THEORY.md" "$staged_bundle/docs/"
if [[ -f "$repo_dir/docs/VALIDATION.md" ]]; then cp "$repo_dir/docs/VALIDATION.md" "$staged_bundle/docs/"; fi

# Include the rebuildable project and its complete pinned JUCE dependency.
# git archive includes tracked source/notices, without checkout history or caches.
source_bundle="$staged_bundle/source"
mkdir -p "$source_bundle/plugin" "$source_bundle/docs" "$source_bundle/.deps/JUCE"
for directory in src tests scripts max; do
    /usr/bin/ditto "$project_dir/$directory" "$source_bundle/plugin/$directory"
done
cp "$project_dir/CMakeLists.txt" "$project_dir/README.md" "$source_bundle/plugin/"
cp "$repo_dir/LICENSE" "$repo_dir/README.md" "$source_bundle/"
cp "$repo_dir/docs/CONTRIBUTING.md" "$repo_dir/docs/THIRD_PARTY_NOTICES.md" "$source_bundle/docs/"
/usr/bin/ditto "$repo_dir/docs/legal" "$source_bundle/docs/legal"
cp "$repo_dir/docs/MUSIC_THEORY.md" "$source_bundle/docs/"
if [[ -f "$repo_dir/docs/VALIDATION.md" ]]; then cp "$repo_dir/docs/VALIDATION.md" "$source_bundle/docs/"; fi
if [[ -f "$repo_dir/.gitignore" ]]; then cp "$repo_dir/.gitignore" "$source_bundle/"; fi

git -C "$juce_source" archive "$juce_commit" | /usr/bin/tar -xf - -C "$source_bundle/.deps/JUCE"
printf '%s\n' "$juce_commit" > "$source_bundle/.deps/JUCE-COMMIT.txt"
cp "$juce_source/LICENSE.md" "$staged_bundle/licenses/JUCE-LICENSE.md"
while IFS='|' read -r target relative; do
    if [[ -f "$juce_source/$relative" ]]; then cp "$juce_source/$relative" "$staged_bundle/licenses/$target"; fi
done <<'LICENSES'
VST3-LICENSE.txt|modules/juce_audio_processors_headless/format_types/VST3_SDK/LICENSE.txt
FLAC-LICENSE.txt|modules/juce_audio_formats/codecs/flac/Flac Licence.txt
Ogg-Vorbis-LICENSE.txt|modules/juce_audio_formats/codecs/oggvorbis/Ogg Vorbis Licence.txt
JPEG-README.txt|modules/juce_graphics/image_formats/jpglib/README
PNG-LICENSE.txt|modules/juce_graphics/image_formats/pnglib/LICENSE
zlib-README.txt|modules/juce_core/zip/zlib/README
HarfBuzz-COPYING.txt|modules/juce_graphics/fonts/harfbuzz/COPYING
SheenBidi-LICENSE.txt|modules/juce_graphics/unicode/sheenbidi/LICENSE
AudioUnitSDK-LICENSE.txt|modules/juce_audio_plugin_client/AU/AudioUnitSDK/LICENSE.txt
LICENSES

(
    cd "$staged_bundle"
    /usr/bin/shasum -a 256 Submotif.vst3/Contents/MacOS/Submotif max/Submotif.amxd \
        max/undertow.js max/undertow-core.js max/undertow-roll.js > SHA256SUMS.txt
)
/usr/bin/ditto -c -k --sequesterRsrc --keepParent "$staged_bundle" "$stage_dir/$bundle_name.zip"
# Replace only this script's generated release outputs, after staging succeeds.
rm -rf "$dist_dir/$bundle_name"
mv "$staged_bundle" "$dist_dir/$bundle_name"
mv -f "$stage_dir/$bundle_name.zip" "$dist_dir/$bundle_name.zip"
echo "Package: $dist_dir/$bundle_name.zip"
