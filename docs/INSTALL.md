# Install Submotif

Download from [the latest GitHub release](https://github.com/habeebmh/submotif/releases/latest). Every ZIP includes the VST3, the Max for Live device, an optional standalone MIDI inspection app, matching source and license notices.

| Download | System |
| --- | --- |
| `Submotif-macos-universal.zip` | macOS 11+, Apple Silicon or Intel (recommended Mac download) |
| `Submotif-macos-arm64.zip` | macOS 11+, Apple Silicon only |
| `Submotif-macos-x86_64.zip` | macOS 11+, Intel only |
| `Submotif-windows-x64.zip` | Windows 10/11, x64 |

Extract the whole ZIP before using it. The `.sha256` file beside each download is its SHA-256 checksum. `BUILD.json` records the tested build; `RELEASE.json` identifies its release tag and commit. `source/` includes the corresponding project and pinned JUCE source.

## Ableton: one track with Max for Live

Requires Live Suite or Live Standard with Max for Live, and Max 9.1.3+. Copy the entire `max` folder into your User Library or add it to Places. Keep `Submotif.amxd` and its three adjacent JavaScript files together.

Load **Submotif.amxd → Serum (or another instrument)** on a MIDI track. Choose a style and key, press Generate, enable Live MIDI, and start playback. Use Create Clip to put the notes into an empty Session clip slot. The instrument supplies all sound.

## VST3 installation

- **Mac:** copy the complete `Submotif.vst3` bundle to `~/Library/Audio/Plug-Ins/VST3/` (create the folder if needed). Enable VST3 system folders in your DAW, then rescan plugins.
- **Windows:** copy the complete `Submotif.vst3` folder to `C:\Program Files\Common Files\VST3\`. You may need administrator permission. Enable VST3 in your DAW and rescan. If Windows reports a missing MSVC runtime DLL, install the [Microsoft Visual C++ v14 x64 Redistributable](https://learn.microsoft.com/en-us/cpp/windows/latest-supported-vc-redist?view=msvc-170) from Microsoft.

In Ableton the VST3 requires **two MIDI tracks**: load Submotif on track 1 and your instrument on track 2. On track 2 select track 1, then Submotif, under MIDI From and set Monitor to In. Enable the generator's MIDI output and start playback. Dropping the VST3 onto an existing instrument can replace it; use the Max edition for a single-track chain. Other DAWs must expose VST3 MIDI output; routing varies by host.

## Signing and validation

These initial releases are **not notarized or Developer ID signed on macOS** (they use ad-hoc signatures). Windows binaries are **not Authenticode signed**. Your operating system or host may block them or display an unidentified-developer warning. No signed installer is included.

The standalone app is for generating, viewing and exporting MIDI; it contains no synthesizer. Automated native, engine, editor and cross-edition checks run for every release target. These checks do not establish compatibility with every DAW; see [validation scope](https://github.com/habeebmh/submotif/blob/main/docs/VALIDATION.md).

To uninstall, remove the Submotif VST3 bundle and the Max folder you installed. Existing projects may report the missing device; exported MIDI clips remain ordinary MIDI.
