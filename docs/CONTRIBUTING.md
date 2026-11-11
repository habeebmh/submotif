# Contributing to Submotif

Help make Submotif more musical, reliable and easier to use. You do not need to write code: a precise listening note, a reproducible bug report, a clearer instruction or a careful DAW test can move the project forward.

The [source](https://github.com/habeebmh/submotif), [issue tracker](https://github.com/habeebmh/submotif/issues) and community pull requests are public. Official downloads are free. Check [GitHub Releases](https://github.com/habeebmh/submotif/releases/latest) for current builds and platform availability.

## What open source means here

Submotif's original code uses **AGPL-3.0-only**. You may use, study, modify, fork and redistribute it, including commercially, under the license's conditions. Sharing covered software also brings obligations around license notices and corresponding source; AGPL section 13 covers source access for people using a modified version over a network. Read the [license guide](legal/license-guide.md) and [full license](../LICENSE) before redistributing a build or operating a modified network service.

Open source includes permission to change and share the software. The free official-download policy is separate from the license and does not prohibit other people from selling compliant copies. You do not have to contribute, pay or follow anyone to use Submotif.

The project claims no ownership of your generated MIDI or music and charges no royalties for ordinary musical output. Using the generator does not automatically place your track or DAW session under the software license. Instruments, samples and other third-party material have their own terms.

## Choose a way to help

| If you enjoy… | A useful first contribution | Where to start |
| --- | --- | --- |
| Making music | Describe a phrase that feels wrong and the musical result you expected. | [Suggest an improvement](https://github.com/habeebmh/submotif/issues/new?template=feature_request.yml) |
| Testing tools | Reproduce a bug or check playback, export and session recall in your DAW. | [Report a bug](https://github.com/habeebmh/submotif/issues/new?template=bug_report.yml) |
| Design and accessibility | Identify a confusing control, clipped layout, keyboard problem or unclear label. | [Browse issues](https://github.com/habeebmh/submotif/issues) |
| Writing or translating | Correct a setup step, explain a host workflow or propose a translated guide. | [Documentation guidance](#documentation-design-and-accessibility) |
| Development | Fix a focused issue in the generator, Max device, interface, website or build tools. | [Your first pull request](#make-a-pull-request) |

Search existing issues before opening one. Confirming another report on your setup or improving its reproduction steps is useful too. Add evidence to that issue rather than opening a duplicate. If you are unsure where to start, describe the area you can help with in an issue; there is no guaranteed assignment or response time.

For a substantial feature or a change to musical behavior, discuss the problem and proposed workflow in an issue first. Small fixes and documentation corrections can go directly to a pull request. Report security vulnerabilities through the [security policy](SECURITY.md), rather than a public bug report.

## Give useful musical feedback

Use the feature form for musical preferences or proposed behavior; use the bug form when a control, constraint or export behaves incorrectly. Include:

- The version and edition, style, key, scale, register, phrase length, and relevant timing or pitch settings. Mention any locked notes.
- The exact exported MIDI that demonstrates the problem, or a small saved example you have permission to share. A seed or saved generator state helps if available. Settings alone cannot reproduce the same notes because each Generate makes a new phrase.
- What you hear: for example, an unstable phrase ending, a disconnected second bar, a weak offbeat pulse or too many large pitch jumps. Describe the intended behavior specifically.
- The instrument and tempo. A brief original audio example can help, but do not upload commercial presets, sample packs or someone else's recordings.

For algorithm proposals, compare several fixed seeds across affected styles and settings, using the same instrument and tempo before and after. Explain the tradeoff: more repetition may improve continuity but reduce variety. Cite primary music-theory or rhythm research where it supports a rule, and identify authored probabilities as design choices. A single pleasing example or a passing test is not proof of musical quality.

## Test in a DAW

Use a small test session. Record the exact OS, processor, DAW, Submotif edition/version and Max version when applicable.

1. Load the device and confirm routing. In Ableton, the Max MIDI device goes before the instrument on one track; the VST3 uses two MIDI tracks.
2. Generate a phrase, start and stop transport, change the loop position, and disable output. Listen for missed or stuck notes.
3. Try the affected MIDI export, drag-out or Create Clip workflow. Check the receiving clip's notes and loop length.
4. Save, close and reopen the session. Check that the phrase, locks and settings return correctly.
5. Report what passed, what failed and what you did not test. Give the shortest reproduction steps, expected behavior and actual result. Remove personal information from logs and project files.

The [validation record](VALIDATION.md) distinguishes automated checks from observed DAW behavior. Do not describe an untested host as supported. Testing a host-specific issue in the actual DAW matters even when unit tests pass.

## Documentation, design and accessibility

Small spelling fixes, broken links and clearer instructions can go straight to a PR, including through GitHub's web editor. A setup guide should name the edition, host and version, give the steps in order, and distinguish tested behavior from assumptions. For translations, start an issue identifying the language and page so the scope and future updates can be discussed.

For interface changes, show the existing problem and proposed result. Include a relevant screenshot and describe keyboard or focus behavior when affected. Check default and expanded plugin views. Website changes should be checked on desktop and narrow phones, including keyboard navigation. Keep the interface simple and readable; new controls or motion should serve a clear user need.

## Make a pull request

1. Fork the repository to your GitHub account. You do not need write access to the main repository.
2. Create a branch in your fork for one change. Update affected instructions and tests.
3. Run the relevant checks below. State which host checks you completed and which you could not run.
4. Open a PR against `habeebmh/submotif:main`. Explain the problem, the resulting behavior and your evidence. Include before/after screenshots or MIDI/audio examples when relevant.
5. Respond to review and push follow-up commits to the same branch. Community PRs require **@habeebmh's approval** before merging; passing checks alone do not approve a change.

After creating your fork, a typical local setup is:

```sh
git clone https://github.com/YOUR_USERNAME/submotif.git
cd submotif
git remote add upstream https://github.com/habeebmh/submotif.git
git switch -c fix/describe-your-change
```

Replace `YOUR_USERNAME` with your GitHub username. Commit and push to your fork's branch, then open the PR. A draft PR is useful for work in progress; say which part needs feedback. Link the relevant issue, using `Fixes #123` when the change actually resolves it. State untested behavior plainly. Do not include release binaries or create release tags as part of a contribution; maintainers handle releases separately.

## What happens during review

Automated checks look for regressions, and Habeeb reviews community PRs for musical behavior, compatibility, usability and scope. Feedback may ask you to simplify a change, add evidence, update both engines or split unrelated work. Reply to the comments and push follow-up commits; the same PR updates automatically.

Contributors retain authorship in Git history. A PR does not guarantee acceptance or inclusion in the next release, and there is no guaranteed review schedule. An issue discussion helps establish direction before a large investment of time. Be kind and specific, discuss the work, and avoid personal attacks or harassment.

## Find the right code

| Location | Responsibility |
| --- | --- |
| `plugin/src/PatternEngine.*` | Portable C++ generation rules and pattern data. |
| `plugin/src/PluginProcessor.*` | VST3 state, MIDI playback and export. |
| `plugin/src/PluginEditor.*` | Native editor. |
| `plugin/max/undertow-core.js` | Matching JavaScript generation engine. |
| `plugin/max/undertow.js`, `plugin/max/undertow-roll.js` | Max integration and piano roll. |
| `plugin/max/build_device.py` | Generates the Max device and readable patch. |
| `plugin/tests/` | Engine, host integration, Max and cross-edition checks. |
| `docs/MUSIC_THEORY.md` | Musical rationale and primary sources. |
| `web/` | Landing page and its assets. |

The C++ and JavaScript generators must stay in agreement. Musical changes usually need matching edits in both engines and fixed-seed parity checks. Preserve saved patterns, parameter identifiers and MIDI export behavior unless a migration is explicitly part of the change.

## Run the relevant checks

Portable checks need CMake 3.22+, a C++17 compiler, Node.js and Python 3; JUCE, Max and a DAW are not required:

```sh
cmake -S plugin -B plugin/build-core -DUNDERTOW_CORE_ONLY=ON -DCMAKE_BUILD_TYPE=Release
cmake --build plugin/build-core --parallel
ctest --test-dir plugin/build-core --output-on-failure
node plugin/tests/max-tests.js
python3 plugin/tests/generation-parity.py
```

The parity script defaults to `clang++`; add `--compiler g++` for GCC. It compiles a temporary adapter. GitHub's **Portable checks** job runs this group on Linux.

Native plugin or editor changes also need an appropriate full build. See [BUILDING.md](BUILDING.md) for prerequisites, architectures and output:

```sh
# macOS: Intel and Apple Silicon in one build
python3 plugin/scripts/build.py --target macos-universal

# Windows: run on Windows with the documented build tools
py -3.12 plugin/scripts/build.py --target windows-x64
```

These include native, Max and parity checks. You do not need both systems to propose a change: report your tested platform and leave other-platform results to native CI or another tester. Do not claim cross-platform validation from a single local build.

For Max patch changes, run `python3 plugin/max/build_device.py` and include the generated AMXD and readable patch. For build, tag or packaging changes, also run the corresponding focused checks:

```sh
python3 -B plugin/tests/build-tests.py
python3 -B plugin/tests/release-tag-tests.py
python3 -B plugin/tests/package-release-tests.py
```

Documentation-only changes need instruction and link checks, not a native build. Website changes need desktop/mobile browser checks and screenshots. Add a regression test when a behavior change needs one, and explain checks that cannot reasonably be automated.

## Keep contributions reviewable

Include only source, tests and necessary assets. Keep build products, caches, credentials, private sessions and personal paths out of commits. Do not add samples, recordings, fonts, images or code unless their license permits inclusion and you document attribution and redistribution requirements. Avoid unrelated reformatting, and regenerate files with their generator rather than editing generated output by hand.

## Contribution license and optional support

By submitting an original contribution for inclusion, you license it under **AGPL-3.0-only**, the same license as Submotif's original code. You retain your copyright. No separate contributor license agreement (CLA) or copyright assignment is required. Identify any third-party material, its compatible license and required notices; contribute only material you have the right to license.

See [LICENSE](../LICENSE), the [license guide](legal/license-guide.md) and [third-party notices](THIRD_PARTY_NOTICES.md) for the full terms. Official downloads are free, and contributing is optional.

To support the project without opening an issue or PR, share it with another producer or [follow Habeeb on Instagram](https://www.instagram.com/habeeeeeeeeeeeeeeeb/). Neither is required, and neither changes the license or review process.
