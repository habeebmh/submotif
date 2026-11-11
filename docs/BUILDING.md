# Build Submotif

`plugin/scripts/build.py` configures, compiles and tests the VST3, standalone inspection application, and Max engine. It never installs a plugin, creates a GitHub Release or publishes the website. Public release packaging is a separate explicit step.

## Targets

| Target | Contains | Build machine |
| --- | --- | --- |
| `macos-arm64` | Apple Silicon VST3 and app | Mac |
| `macos-x86_64` | Intel VST3 and app | Mac |
| `macos-universal` | Both Intel and Apple Silicon slices in each VST3/app | Mac |
| `windows-x64` | 64-bit Windows VST3 and EXE | Windows |

A universal Mac build is usually the convenient choice: one plugin works on either Mac architecture. Windows ARM and 32-bit Windows are not targets. macOS deployment minimum remains 11.0. A successful build does not establish compatibility with every DAW.

## Prerequisites

- Python 3.12+, CMake 3.22+, Git and Node.js 22+.
- Mac: Xcode Command Line Tools (`xcode-select --install`). Ninja is optional.
- Windows: Visual Studio 2022 or its Build Tools, with **Desktop development with C++**, the MSVC v143 x64 tools and a Windows SDK. The script uses the Visual Studio CMake generator, so a Developer Command Prompt is not required.
- The first configure needs network access if the pinned JUCE checkout is absent from `.deps/JUCE`. The build uses the JUCE revision specified in `plugin/CMakeLists.txt`.

## Local commands

From the repository root, on a Mac:

```sh
python3 plugin/scripts/build.py --target macos-universal
python3 plugin/scripts/build.py --target macos-arm64
python3 plugin/scripts/build.py --target macos-x86_64
```

Or build all three Mac variants sequentially:

```sh
python3 plugin/scripts/build.py --target macos-all
```

On Windows, in PowerShell or Command Prompt:

```powershell
py -3.12 plugin/scripts/build.py --target windows-x64
```

With no `--target`, the script chooses universal macOS or x64 Windows according to the host. It refuses to build Windows on macOS, or macOS on Windows. Use the GitHub workflow below for all platforms from one request.

Build output is isolated in `plugin/build-<target>/`. Inside it:

- `Undertow_artefacts/Release/VST3/Submotif.vst3`
- `Undertow_artefacts/Release/Standalone/Submotif.app` on Mac, or `Submotif.exe` on Windows
- `BUILD.json`, including target, architectures, source revision, binary hashes and test/signing status

`Undertow` remains an internal target name for compatibility. The standalone app is for inspection and has no built-in bass synthesizer. The platform-independent Max device remains in `plugin/max/`; keep its companion files together.

## Tests and architecture checks

Each normal build runs CTest, Max checks, audit-metric tests and C++/Max parity. The parity probe is built by CMake with the same compiler and architecture as the plugin, including MSVC on Windows. macOS additionally tests the actual VST3 wrapper; that harness currently uses Apple's bundle-loading API and is not run on Windows.

The script checks both Mac binaries with `lipo`, then signs and verifies the completed bundles with an ad-hoc development signature. On Windows it checks the PE machine header of both binaries for x64. Wrong-architecture caches are rejected rather than silently reused.

Tests need an executable host architecture. Running Intel-only tests on Apple Silicon requires Rosetta; ARM-only tests cannot execute on Intel. Use native GitHub runners for verification. `--skip-tests` allows an explicit compile-only local build, records it as untested, and cannot be combined with `--artifact`.

Other options:

```sh
python3 plugin/scripts/build.py --target macos-universal --jobs 4
python3 plugin/scripts/build.py --target macos-arm64 --core-only
python3 plugin/scripts/build.py --target macos-universal --dry-run
```

The old `build-macos.sh` entry point remains compatible: it builds Apple Silicon into `plugin/build/`, or the portable engine into `plugin/build-core/` with `--core-only`. The legacy Mac release packager still expects that location.

## Build every platform on GitHub

The **Native builds** workflow runs on relevant pull requests and main-branch changes. Once merged into the default branch it can also be run from **Actions → Native builds → Run workflow**. Four jobs build Apple Silicon, Intel, universal Mac, and Windows x64, using native ARM Mac, Intel Mac and Windows runners.

Each successful job uploads a `Submotif-<target>-development` artifact for seven days. Its tar.gz preserves Mac permissions and includes the VST3, inspection app/EXE, Max device, build manifest, checksums, project source and complete pinned JUCE source with its notices. These are development artifacts, not approved releases or installers. Mac builds are not Developer ID signed/notarized; Windows builds are not Authenticode signed.

The same artifact can be created locally after tests:

```sh
python3 plugin/scripts/build.py --target macos-universal --artifact
```

Artifacts go to `dist/builds/`. A source-inclusive artifact requires the clean pinned JUCE checkout used by the build. It includes actual working-tree plugin source, and `BUILD.json` reports whether the working tree was dirty. Do not edit source while a build is running. Unversioned source copies can be rebuilt without Git history; their manifest reports an unknown revision/dirty state. Source fingerprints prevent artifacts from being produced if plugin code changes during the build or source copy. The full Max folder is architecture-independent and included with each target for convenience.

References: [CMake Mac architectures](https://cmake.org/cmake/help/latest/variable/CMAKE_OSX_ARCHITECTURES.html), [CMake Visual Studio x64 selection](https://cmake.org/cmake/help/latest/generator/Visual%20Studio%2017%202022.html), [GitHub runner labels](https://docs.github.com/en/actions/how-tos/write-workflows/choose-where-workflows-run/choose-the-runner-for-a-job).

## Release tags

Owner-directed work happens directly on `main`. External contributions can still use pull requests. Every release must have an immutable **annotated** tag `vMAJOR.MINOR.PATCH`, matching the version in `plugin/CMakeLists.txt` and pointing to a commit in main's history.

Release publication requires an explicit owner request. To inspect the next tag without creating one:

```sh
python3 plugin/scripts/release-tag.py --dry-run
```

After approval, update the CMake version, commit and push main, and wait for its native checks to pass. Then create and push the annotated tag:

```sh
python3 plugin/scripts/release-tag.py --create --push
```

The script refuses dirty/non-main checkouts, an existing tag, or a main commit that differs from the remote. Tag pushes trigger **Release builds** for all four native targets; CI rejects lightweight tags, version mismatches and commits outside main. Tags are never force-moved. The workflow creates a draft GitHub Release and uploads source-inclusive ZIPs directly, without relying on temporary Actions artifact downloads. It never publishes the draft automatically.

Each target runs all applicable native and cross-edition checks, then `package-release.py --target <target> --tag vMAJOR.MINOR.PATCH` validates the tested commit, clean working tree, source fingerprints, archive checksums, binary hashes and architecture. Mac signatures are verified again. ZIPs preserve executable modes and symlinks, include installation instructions, and have adjacent SHA-256 and provenance files. Asset names stay stable so the website can use `/releases/latest/download/<filename>`.

After every target passes and all twelve assets (four ZIPs, four checksum files and four provenance files) are verified, publish the draft with `gh release edit vMAJOR.MINOR.PATCH --draft=false --latest`. The installation guide records the current signing status. Do not overwrite published assets; corrections require a new version and tag.
