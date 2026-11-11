# SUBMOTIF by Habeeb

**A fresh bassline. Your sound.**

SUBMOTIF is a free, open-source MIDI bassline generator for house music. Choose a style, set your key, and generate a phrase for your favorite instrument. Keep what works, change a few notes, and take the MIDI into your track.

![SUBMOTIF's compact editor with style, key, scale and length controls above a MIDI piano roll](web/assets/images/submotif-editor.png)

**Version 1.2.2:** the universal Mac download supports Apple Silicon and Intel and includes Max for Live. See the release page for Windows and separate-architecture Mac availability. [Download the latest release](https://github.com/habeebmh/submotif/releases/latest) and read the [installation guide](docs/INSTALL.md). Mac builds are not notarized; Windows builds are unsigned.

[Website](https://submotif.web.app) · [GitHub](https://github.com/habeebmh/submotif) · [Latest release](https://github.com/habeebmh/submotif/releases/latest) · [Contribute](docs/CONTRIBUTING.md) · [Report an issue](https://github.com/habeebmh/submotif/issues)

Listen to [six selected examples](https://submotif.web.app/#showcase), including Acid through Serum at the approved higher register. These recordings demonstrate MIDI played through instruments; the plugin itself supplies MIDI only.

## Make it yours

Every **Generate** runs the algorithm again. Weighted choices shape a rhythmic motif, assign harmonic roles, and develop a repeating phrase. There is no bank of premade basslines.

- **Six styles:** Deep House, Rolling, Offbeat, Acid, Disco, and Funk.
- **Your instrument supplies the sound.** SUBMOTIF generates MIDI; it has no built-in synthesizer.
- **A simple starting point:** C minor, C1, and one bar. Choose 1, 2, 4, or 8 bars in 4/4.
- **Keep the good parts:** lock notes, make a small variation, undo changes, and save the exact pattern with your project.
- **Keep the MIDI:** drag a pattern from the VST3, create a Live clip from the Max device, or save a MIDI file.

The generation rules draw on research into groove, tonal stability, and melodic continuity. Style weights are design choices, not measurements attributed to those papers. Read the [music theory notes and sources](docs/MUSIC_THEORY.md).

## Choose your edition

| Edition | How it fits your session | Requirements |
| --- | --- | --- |
| **VST3** | Sends MIDI to your instrument. In Ableton Live, use two MIDI tracks. | Build targets: Intel/Apple Silicon macOS 11+ and Windows x64, in a VST3 host that exposes plug-in MIDI output. |
| **Max for Live** | Goes directly before your instrument on one Ableton MIDI track. | Live Suite, or Live Standard with Max for Live; Max 9.1.3+. |

Native build automation covers Intel Mac, Apple Silicon, universal Mac and Windows x64. Broader DAW compatibility testing remains pending. See [validation notes](docs/VALIDATION.md) for completed checks and remaining host testing. A passing automated test is not a compatibility guarantee.

Official releases are free to download from [GitHub Releases](https://github.com/habeebmh/submotif/releases/latest). No payment, follow, or contribution is required. Check the release notes for supported systems, installation instructions, and signing status before installing a build.

## Start in Ableton

### Max for Live: one track

For Serum on the same Ableton track, use **Submotif.amxd → Serum → audio effects**. Load the Max MIDI device from your User Library. Live treats the VST3 edition as an instrument, so dropping that edition onto Serum can replace Serum; its routing uses the separate track described below.

1. Keep `Submotif.amxd`, `undertow.js`, `undertow-core.js`, and `undertow-roll.js` together in one folder. Add that folder to Live's **Places** or copy it into your User Library.
2. Drag **Submotif.amxd** onto a MIDI track, before your bass instrument.
3. Choose a style and key, press **Generate**, enable **Live MIDI**, and start Live's transport.
4. To keep a pattern, select a scene with an empty clip slot on that track and press **Create Clip**. SUBMOTIF switches live generation off. Launch the new clip to hear it.

Create Clip never overwrites an existing clip. **Save MIDI** exports a file instead. See the [Max guide](plugin/max/README.md) for setup and timing details; playing an exported clip uses Live's own MIDI scheduling.

### VST3: two tracks

After installing the VST3, enable VST3 folders in Live's plug-in settings and rescan if necessary. Find **SUBMOTIF** under manufacturer **Habeeb**.

1. Put SUBMOTIF on an empty MIDI track and your bass instrument on a second MIDI track.
2. On the instrument track, set **MIDI From** to SUBMOTIF's track, choose **SUBMOTIF** in the lower chooser, and set **Monitor → In**.
3. Press **Generate**, enable **Live MIDI**, and start the transport.
4. Drag **Drag MIDI** onto the instrument track to keep the phrase. Turn SUBMOTIF's **Live MIDI off**, set the instrument track's **Monitor → Auto**, and play the clip.

Clicking **Drag MIDI**, or choosing **Help → Save MIDI**, opens a file export. This setup follows [Ableton's MIDI-output routing instructions](https://help.ableton.com/hc/en-us/articles/209070189-Accessing-the-MIDI-output-of-a-VST-plug-in). Use the Max edition when you want the generator and instrument on the same Live track.

## Shape a phrase

| Control | What it does |
| --- | --- |
| **Style, Key, Scale, Length** | Set the phrase's musical direction. Press Generate to apply changes. Scales include natural minor, Dorian, Phrygian, major, and minor pentatonic. |
| **Density** | Changes how many notes play. |
| **Melody** | Changes how much the pitches move. |
| **Change a few notes** | Keeps the rhythm and most notes while making a small variation. Apply pending settings with Generate first. |
| **Click a note** | Locks or unlocks it. Locked notes survive generation and variation. Shortening a loop removes locks that no longer fit. |
| **More controls** | Reveals Swing, Note length, Register, Root notes only, Start on root, and Octave jumps. |

**Root notes only** keeps new notes on the tonic pitch class; **Octave jumps** permits its upper octave. **Start on root** makes the first sounded note the tonic when that note is unlocked, without moving it onto the downbeat. Locks take priority over new pitch settings. C1 uses Ableton's octave convention: MIDI note 36.

## Repository layout

| Folder | Contents |
| --- | --- |
| [`plugin/`](plugin/README.md) | VST3 source, Max for Live device, tests, and build tools. |
| [`web/`](web/README.md) | Landing page, browser demo, approved examples, and site tools. |
| [`docs/`](docs/README.md) | Guides, contribution and security policies, licenses, research, and design records. |

GitHub automation stays in `.github/`. Build and test commands below run from the repository root.

## Build from source

The shared build script supports Intel Mac, Apple Silicon, universal Mac and Windows x64. Install the prerequisites in [BUILDING.md](docs/BUILDING.md), then run on the corresponding OS:

```sh
# Mac: one VST3/app containing both Intel and Apple Silicon code
python3 plugin/scripts/build.py --target macos-universal

# Windows: x64 VST3 and inspection app
py -3.12 plugin/scripts/build.py --target windows-x64
```

Use `macos-arm64` or `macos-x86_64` for a single Mac architecture. The **Native builds** GitHub workflow builds and tests all four targets and uploads temporary development artifacts. Output folders, dependencies, test coverage and signing status are documented in the [build guide](docs/BUILDING.md). Annotated version tags trigger the separate release workflow, which uploads verified ZIPs to a draft GitHub Release for explicit publication.

JUCE is pinned in `plugin/CMakeLists.txt`. The first configuration fetches it when `.deps/JUCE` is absent, so internet access is needed for that step. Review [third-party notices](docs/THIRD_PARTY_NOTICES.md) before redistributing a build.

For portable generation checks without JUCE, Max, or a DAW:

```sh
cmake -S plugin -B plugin/build-core -DUNDERTOW_CORE_ONLY=ON -DCMAKE_BUILD_TYPE=Release
cmake --build plugin/build-core --parallel
ctest --test-dir plugin/build-core --output-on-failure
node plugin/tests/max-tests.js
python3 plugin/tests/generation-parity.py
```

The parity check uses Clang by default; pass `--compiler g++` to use GCC. See [CONTRIBUTING.md](docs/CONTRIBUTING.md) for the code layout and contribution workflow.

## Help shape SUBMOTIF

Submotif is open-source software under AGPLv3: you can study, modify and share the code under its terms. Official downloads are free. You do not need to code, pay or follow anyone to get involved.

- **Musicians:** share an original MIDI example, the settings and what you would change about the groove or melody. [Send musical feedback](https://github.com/habeebmh/submotif/issues/new?template=feature_request.yml).
- **Testers:** check playback, MIDI export and session recall in your DAW. Include your exact setup and reproduction steps in a [bug report](https://github.com/habeebmh/submotif/issues/new?template=bug_report.yml).
- **Developers and designers:** fix a focused issue, improve accessibility or propose a clearer interface. Discuss larger changes before implementation and keep the native and Max engines in agreement.
- **Writers and translators:** correct instructions, document a tested host workflow or propose a translated guide. Small documentation fixes are welcome.

Start with [existing issues](https://github.com/habeebmh/submotif/issues) and the [contribution guide](docs/CONTRIBUTING.md). It covers first pull requests, musical evidence, DAW testing, relevant checks and licensing. Fork the repository, make a focused change and open a PR against `main`; community PRs require **@habeebmh's approval** before merging. Contributors retain their copyright and contribute under the same AGPLv3-only license.

If SUBMOTIF helps you make something, an optional [follow on Instagram](https://www.instagram.com/habeeeeeeeeeeeeeeeb/) is a welcome thank-you. Sharing feedback or contributing to the project is welcome too.

## License

SUBMOTIF's original code is licensed under the **GNU Affero General Public License, version 3 only (`AGPL-3.0-only`)**. You may modify it, create forks, and redistribute or sell copies under the license's conditions, including applicable source-sharing requirements. The free official-download policy does not restrict those rights. Read [LICENSE](LICENSE), the [license guide](docs/legal/license-guide.md), and [third-party notices](docs/THIRD_PARTY_NOTICES.md) for the full terms and dependency licenses.

Contributors retain their copyright and contribute under the same AGPLv3-only license. No separate contributor license agreement or copyright assignment is required.

---

[Terms of use](docs/legal/terms.md) · [Liability](docs/legal/liability.md) · [Privacy](docs/legal/privacy.md) · [AGPLv3 license](LICENSE) · [Security](docs/SECURITY.md)
