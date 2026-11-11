# Contributing to SUBMOTIF

Thanks for helping make a better bassline generator. Useful contributions include clearer instructions, reproducible bug reports, host compatibility checks, accessibility improvements, and focused musical or technical changes.

**Preview status:** source and contributions are open; release packaging remains on hold pending review. The project uses AGPLv3-only.

## Start with a small, clear change

Search [existing issues](https://github.com/habeebmh/submotif/issues) before opening a new one. For a substantial feature or a change to musical behavior, describe the problem and proposed workflow in an issue first. Documentation corrections and small fixes can go directly to a pull request.

For a bug, include the SUBMOTIF version and edition, operating system and processor type, DAW version, and Max version if applicable. Give the shortest steps that reproduce it, the expected behavior, and what actually happened. A short MIDI example or screen recording can help; share only material you have permission to publish. Remove personal information from logs and project files.

Report security vulnerabilities through the [security policy](SECURITY.md), rather than a public bug report.

## Make a pull request

1. Fork the repository and create a branch for one change.
2. Make the change and update affected instructions or tests.
3. Run the relevant checks below. State which host checks you completed and which you could not run.
4. Open a pull request against `main`. Explain the problem, the resulting behavior, and the evidence that it works. Add screenshots for visible changes and before/after MIDI or audio examples for musical changes when useful.
5. Respond to review and keep changes focused. The intended merge policy requires approval from **@habeebmh**. A passing check alone does not approve a change.

Be kind and specific in issues and reviews. Discuss the work, respect contributors' time, and avoid personal attacks or harassment. There is no guaranteed review or release schedule.

## Find the right code

| Location | Responsibility |
| --- | --- |
| `Source/PatternEngine.*` | Portable C++ generation rules and pattern data. |
| `Source/PluginProcessor.*` | VST3 state, MIDI playback, and export. |
| `Source/PluginEditor.*` | Native editor. |
| `max/undertow-core.js` | Matching JavaScript generation engine. |
| `max/undertow.js`, `max/undertow-roll.js` | Max integration and piano roll. |
| `max/build_device.py` | Generates the Max device and readable patch. |
| `tests/` | Engine, host integration, Max, and cross-edition checks. |
| `docs/MUSIC_THEORY.md` | Musical rationale and primary sources. |
| `site/` | Landing page and its assets. |

The C++ and JavaScript generators must stay in agreement. Musical changes usually need matching edits in both engines and a fixed-seed parity check. Preserve saved patterns, existing parameter identifiers, and MIDI export behavior unless a migration is explicitly part of the change.

For musical rules, cite relevant primary sources and explain which numerical choices are your own design decisions. Listen across multiple seeds and styles; theory and passing tests do not establish that a phrase sounds good.

## Run the checks

Portable checks require CMake 3.22+, a C++17 compiler, Node.js, and Python 3. They do not require JUCE, Max, or a DAW:

```sh
cmake -S . -B build-core -DUNDERTOW_CORE_ONLY=ON -DCMAKE_BUILD_TYPE=Release
cmake --build build-core --parallel
ctest --test-dir build-core --output-on-failure
node tests/max-tests.js
python3 tests/generation-parity.py
```

The parity script defaults to `clang++`; use `python3 tests/generation-parity.py --compiler g++` for GCC. It compiles its own temporary test adapter. GitHub's **Portable checks** job runs this group on Linux.

Native plug-in or editor changes also need an Apple Silicon macOS build, with Xcode Command Line Tools installed:

```sh
./scripts/build-macos.sh
```

This runs the native tests in addition to the Max and parity checks. For Max patch changes, regenerate the device with `python3 max/build_device.py` and include the corresponding generated files in the pull request.

When behavior depends on a DAW, test it in the actual host: loading, transport start/stop, MIDI routing, saving and reopening a session, and the affected export or clip workflow. Report the exact host and version. Automated tests cover only part of this; [validation notes](docs/VALIDATION.md) list known gaps. Do not label an untested host as supported.

## Keep contributions reviewable

Include only source, tests, and necessary assets. Keep build products, dependency caches, credentials, private sessions, and personal paths out of commits. Do not add samples, recordings, fonts, images, or code unless their license permits inclusion and you document attribution and redistribution requirements.

Do not change generated files by hand when a generator exists. Avoid unrelated reformatting. Add a regression test when a behavior change needs one, and explain any check that cannot reasonably be automated.

## Contribution license

By submitting an original contribution for inclusion, you license it under the **GNU Affero General Public License, version 3 only (`AGPL-3.0-only`)**, the same license as SUBMOTIF's original code. You retain your copyright. No separate contributor license agreement (CLA) or copyright assignment is required.

AGPLv3 permits modifications, forks, and sales under its conditions; it is not a ban on commercial use or derivative works. Official SUBMOTIF downloads will be free, and contributing is optional. Do not submit material you do not have the right to contribute. Identify any third-party material and its compatible license. See [LICENSE](LICENSE), [the license guide](docs/legal/license-guide.md), and [third-party notices](THIRD_PARTY_NOTICES.md).
