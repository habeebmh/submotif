# Submotif plugin

The VST3 and Max for Live editions share the same weighted bassline generator.

| Folder | Contents |
| --- | --- |
| `src/` | C++ engine, MIDI processor, and native editor. |
| `max/` | Max for Live device, readable patch, JavaScript engine, and device builder. |
| `tests/` | Native, Max, audit, and cross-edition checks. |
| `scripts/` | Cross-platform builds, verified release packaging, and research/audition tools. |

From the repository root, run `python3 plugin/scripts/build.py --target macos-universal` on a Mac, or `py -3.12 plugin/scripts/build.py --target windows-x64` on Windows. Thin Mac targets are `macos-arm64` and `macos-x86_64`. Builds use isolated `plugin/build-<target>/` folders and run native/Max/parity checks. See [BUILDING.md](../docs/BUILDING.md) for prerequisites, all-platform GitHub automation and development artifacts. The old `build-macos.sh` retains its Apple Silicon behavior and paths.

See the [main README](../README.md) for installation and use, the [Max guide](max/README.md), and the [contribution guide](../docs/CONTRIBUTING.md) for checks. See [INSTALL.md](../docs/INSTALL.md) for the released downloads and signing status.


In v1.3.0 source/local builds, **More controls → Octave shift** transposes the current phrase immediately by −5 to +5 octaves. MIDI note limits apply to the whole phrase; invalid shifts are rejected without clipping notes. Playback, export, saved state and Undo/Redo use the same shift. The default remains C1 and one bar.
