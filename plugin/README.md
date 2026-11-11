# Submotif plugin

The VST3 and Max for Live editions share the same weighted bassline generator.

| Folder | Contents |
| --- | --- |
| `src/` | C++ engine, MIDI processor, and native editor. |
| `max/` | Max for Live device, readable patch, JavaScript engine, and device builder. |
| `tests/` | Native, Max, audit, and cross-edition checks. |
| `scripts/` | Cross-platform builds, future release packaging, and research/audition tools. |

From the repository root, run `python3 plugin/scripts/build.py --target macos-universal` on a Mac, or `py -3.12 plugin/scripts/build.py --target windows-x64` on Windows. Thin Mac targets are `macos-arm64` and `macos-x86_64`. Builds use isolated `plugin/build-<target>/` folders and run native/Max/parity checks. See [BUILDING.md](../docs/BUILDING.md) for prerequisites, all-platform GitHub automation and development artifacts. The old `build-macos.sh` retains its Apple Silicon behavior and paths.

See the [main README](../README.md) for installation and use, the [Max guide](max/README.md), and the [contribution guide](../docs/CONTRIBUTING.md) for checks. Release packaging remains on hold.
