# Third-party notices

## SUBMOTIF's original code

Copyright (C) 2026 Habeeb and SUBMOTIF contributors.

SUBMOTIF's original source code is licensed under the **GNU Affero General Public License, version 3 only (AGPL-3.0-only)**. See [LICENSE](LICENSE) for the complete license and the [license guide](docs/legal/license-guide.md) for applicability and contribution information. Third-party components retain their own copyright and license notices below; this project notice does not replace them.

## JUCE

Submotif's VST3 and standalone application use the JUCE Framework, pinned to commit `dabd5ad13cd869cba41046947fffc8e5ce5cd662` (`8.0.17-security`). The checkout's `LICENSE.md` states that JUCE modules are dual-licensed under **AGPLv3 or the commercial JUCE license**. See the [upstream source and license](https://github.com/juce-framework/JUCE/blob/dabd5ad13cd869cba41046947fffc8e5ce5cd662/LICENSE.md), [AGPLv3](https://www.gnu.org/licenses/agpl-3.0.en.html), and [JUCE 8 license](https://juce.com/legal/juce-8-licence/).

The build generates an adapted copy of JUCE's VST3 wrapper using `scripts/cmake/UndertowVST3InputGuard.cmake`. It discards incoming MIDI after conversion while preserving the input bus required by Ableton's instrument host contract. The pinned checkout remains unchanged and the generated copy retains JUCE's notices.

This project uses JUCE's **AGPLv3 licensing route**, alongside the AGPLv3-only license for SUBMOTIF's original code. A commercial JUCE license is not asserted or supplied here.

JUCE includes third-party components with their own notices. The package script retains the JUCE license and available component license files under `licenses/`, including VST3, image/font libraries, compression libraries, and audio codecs. The JUCE license lists the complete dependency set; not every dependency it lists is used on every target.

Release packaging remains on hold pending review. Before distributing a build, include the complete project license, the required dependency notices, and the Corresponding Source required by the AGPL. Dependency source must remain available as required by the license; a dependency name or build instruction alone is not a substitute for that obligation. The packaging script includes the pinned JUCE source and project source, but has not been executed for this version. A release still needs archive-level verification; this notice does not certify an ungenerated archive as compliant.

This software is based in part on the work of the Independent JPEG Group.

## Steinberg VST 3 SDK

The VST3 SDK bundled with the pinned JUCE source uses the following MIT license, reproduced from `modules/juce_audio_processors_headless/format_types/VST3_SDK/LICENSE.txt`:

MIT License

Copyright (c) 2025, Steinberg Media Technologies GmbH

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

## Max for Live and product names

The Max edition requires the user's licensed Ableton Live/Max for Live runtime. Neither application is distributed with Submotif. The AMXD envelope follows the structure of the installed Max MIDI Effect template; the patch, interface, and generator logic are original project work. Ableton, Max for Live, JUCE, and VST are names or marks of their respective owners. No affiliation or endorsement is implied.

Research citations are provided in `docs/MUSIC_THEORY.md`. Reference-product graphics, presets, recorded MIDI, audio samples, and commercial plugin code are not included.

## Website utilities and typeface

The static website includes layout and motion utilities from Leonxlnx's Taste Skill under the MIT license. The full notice is retained in `site/assets/LICENSE-taste-skill.txt`; its source is [the upstream license](https://github.com/Leonxlnx/taste-skill/blob/main/LICENSE). These utilities are separate from the SUBMOTIF generation algorithm.

Host Grotesk is self-hosted under the SIL Open Font License 1.1. The supplied copyright and license are retained in `site/assets/fonts/OFL-Host-Grotesk.txt`.

The plugin screenshots show this project's actual interface. The original audition candidates use freshly generated SUBMOTIF MIDI and project-generated bass/drums. The approved Acid showcase renders the selected pattern, raised by two octaves, through the owner's Serum 2 instrument and requested `TSP_S290V3_Bass_acid_sweep.SerumPreset`. Serum, its preset file, and its saved instrument state are not distributed. The instrument and preset remain subject to their separate licenses. Website playback is an audition aid, not a synthesizer supplied by the plugin.
