# Submotif plugin

The VST3 and Max for Live editions share the same weighted bassline generator.

| Folder | Contents |
| --- | --- |
| `src/` | C++ engine, MIDI processor, and native editor. |
| `max/` | Max for Live device, readable patch, JavaScript engine, and device builder. |
| `tests/` | Native, Max, audit, and cross-edition checks. |
| `scripts/` | macOS build, future packaging, and research/audition tools. |

From the repository root, run `./plugin/scripts/build-macos.sh` for the native build, or `./plugin/scripts/build-macos.sh --core-only` for the portable engine. Build outputs stay in `plugin/build/` or `plugin/build-core/`. JUCE is pinned in [CMakeLists.txt](CMakeLists.txt); an existing repository-level `.deps/JUCE` checkout is reused.

See the [main README](../README.md) for installation and use, the [Max guide](max/README.md), and the [contribution guide](../docs/CONTRIBUTING.md) for checks. Release packaging remains on hold.
