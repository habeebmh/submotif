# Submotif development validation

Paths under `dist/` refer to local development evidence and are not included in the public source checkout. The documented scripts reproduce automated checks; historical host observations are recorded below.

Development validation recorded on November 11, 2026 (UTC) on Apple Silicon (M2 Max), macOS 26.5.2, with Ableton Live 12.4.16 Suite and its Max runtime. **Release packaging is on hold while the product is refined.** This document describes development evidence, not a guarantee that every host or workflow is bug-free.

## 1.2.2 Submotif rename

- Both editions now display **SUBMOTIF**, manufacturer **Habeeb**. Build outputs are `Submotif.vst3` and `max/Submotif.amxd`.
- Persistent VST3 component/controller IDs, bundle ID, state schema, and the exact musical-core source hashes are unchanged. Internal `Undertow` target/source identifiers remain for compatibility.
- The renamed build passes all four CTest suites, 32 Max groups, and 1,260-pattern cross-edition parity. The native editor run passes 1,691 checks; actual screenshots are in `dist/ui-v1.2.2/`.
- VST3 and standalone development signatures verify. This build has not been installed or retested in Live; the historical observations and remaining host checks below still apply.
- Packaging and release publication remain on hold. No release archive was generated.

## 1.2.1 branding update

- VST3 factory and class metadata now report **UNDERTOW**, manufacturer **Habeeb**. Native scan output confirms `Habeeb: UNDERTOW v1.2.1` and plug-in name `UNDERTOW`. Both interfaces display UNDERTOW by Habeeb.
- Persistent component/controller IDs, bundle identifier, and existing file locations are unchanged. The native processor name uses the same build metadata as the VST3 class.
- The full build passes all four native suites, all 32 Max groups and the 1,260-pattern parity comparison. Pluginval strictness 5 passes in `dist/validation-v1.2.1/`. Native interface captures in `dist/ui-v1.2.1/` were checked for header clipping.
- The validated VST3 passes strict/deep ad-hoc signature verification. The Max AMXD was regenerated. No release package was generated.
- Ableton's refreshed browser listing and the remaining host checks below have not been verified for the final 1.2.1 build.

## 1.2 interface revision

- New VST3 and Max instances default to C1 (MIDI 36) and one bar. Explicit settings and exact notes in existing projects remain unchanged.
- Native editor is 920 × 560 by default, with compact selectors and a closely grouped Generate / Change a few notes pair. More controls reveals timing and pitch options. The expanded view fits the display bounds.
- Max uses a 1000 × 169 basic presentation and expands sideways to 1260 × 169. Its parameter names remain compatible with existing Live Sets.
- Changing musical controls leaves the displayed phrase intact. Change a few notes is disabled and guarded until Generate applies pending settings.
- All four native suites pass: engine, processor, editor and native VST3 wrapper. The engine reports 655,143 checks and the editor 1,665. Editor checks cover visibility, focus, pending state, generation/history, locks, output, and resized bounds. Actual native compact/expanded/minimum/large captures are in `dist/ui-final/`. The copied CTest log is in `dist/validation-v1.2-final/ctest.log`.
- All 32 Max groups and the 1,260-pattern cross-edition comparison passed after the UI/default changes. Mocked Max APIs do not establish host rendering correctness.
- Ableton host testing exposed a bus-contract issue inherited from 1.1: Live requires an event input bus for an instrument-category VST3. The final 1.2 build advertises that bus and discards all incoming MIDI. A generated JUCE wrapper guard clears input after conversion, including zero-audio parameter flushes and the suspended path, without modifying the pinned dependency. The native VST3 harness loads the actual bundle and verifies bus activation, generated output, rejection of external note/expression events, disabled output and zero-audio flush. Processor tests separately compare fed and clean output through active, disabled, bypassed and stopped cases.
- Final 1.2 pluginval strictness 5 passes (`dist/validation-v1.2-final/`), after the bus repair. The build and installed copy both pass strict/deep ad-hoc signature checks and have the same binary hash.
- The complete `scripts/build-macos.sh` run passes all four native suites, all 32 Max groups and the 1,260-pattern parity comparison. No release packaging was run.
- In the actual validation Set, the redesigned Max device displayed and generated a one-bar phrase, retained its previously saved Register 2/Funk settings, and expanded its width through More controls. That inspection found a blank advanced area. A redundant foreground panel was identified as the likely occluder, removed, and AMXD rebuilt; the visual effect of this fix still needs host confirmation. Final visual confirmation of those controls, a fresh-instance default check in Live, and loading the final repaired VST3 remain pending.

## Earlier 1.1 automated checks

- Release VST3 and standalone targets build successfully for macOS arm64. The standalone has no preview synthesizer.
- Both CTest suites pass: portable engine and JUCE processor integration. The engine reports 655,547 assertions, including key/scale/register constraints, style variation, deterministic seeds, locks, export structure, and transport scheduling. Address/undefined-behavior sanitizer checks of the portable engine also passed.
- Processor checks cover exact saved-state restoration, malformed state rejection, pending controls versus committed notes, undo/redo, note locks, MIDI export parsed by JUCE, stop/bypass/output cleanup, and the MIDI-output-only host contract.
- All 28 Max test groups pass. They cover generation across 1,440 settings combinations, diversity, register, phrase opening retention, resolving connectors, bounded Variation, nearby locks, modal colors, state, MIDI export, scheduler cleanup, safe clip targeting with a mocked Live API, the AMXD envelope and patch wiring, and momentary action controls that are not stored or automated. They do not replace host testing.
- Independent scheduler review compared 645 render combinations against reference event streams, including varied buffer sizes, sample rates, and tempos. Independent Max stress checks exercised 500 settings transformations and lock preservation.
- Tracktion pluginval 1.0.4 passes at strictness 5 on the 1.1 VST3, including its editor, state, buses, and processing at 44.1/48/96 kHz with buffers of 64/128/256/512/1024 samples. Final 1.1 logs are in `dist/validation-v1.1-final/`. The separate Steinberg validator was not configured or run.
- The built VST3 passes macOS strict/deep signature verification. It is locally ad-hoc signed, not Developer ID signed or notarized.

## Musical revision checks

- `scripts/build-macos.sh` completes the Release build, signs and verifies completed development bundles after manifest generation, passes both CTest suites, passes all 28 Max test groups, and passes `tests/generation-parity.py`.
- The parity test compares 1,260 actual C++/Max outputs over 180 scenarios across six styles/five scales. It includes Generate, three consecutive Variations, regeneration with locks, changed settings, and off-grid/extreme-pitch locks. Counts, pitches, velocities, locks and seeds match exactly; onset/duration differences are at most 1e-12 beat. Unit controls are rounded to the C++ float boundary for generation only; saved values are not quantized to percentages.
- Independent review exercised 40,000 arbitrary-lock/Variation cases and 600 restricted-JavaScript-runtime generation/export cases. The final portable engine also passed address/undefined-behavior sanitizers. These checks do not replace host testing.
- The revised structural audit covers 9,216 base phrases plus one Variation each: 6,144 matched two-bar cases, 1,536 four-bar cases and 1,536 eight-bar cases. [Measured changes](research/2026-11-11-revision-results.md) report repetition, register, connector resolution and density without a subjective quality score.
- `dist/auditions-v1.1/` contains six before/after comparisons, six revised-only WAVs and 36 exact MIDI files. Independent reconstruction matched every audio sample; MIDI matched corpus notes at 960 PPQ. Audio is 44.1 kHz/16-bit/mono with no clipping, identical synthesis/drums/tempo and one shared gain. Each phrase loops twice. Fixed seed indices 0, 1, 2 were selected before inspection; examples are not hand-picked for quality.
- The 1.1.0 development install matched the validated build binary and signature. Existing loaded instances can retain a prior version until the host is restarted.
- Max AMXD was rebuilt with the revised adjacent JavaScript files. The full folder remains necessary. Saved patterns recall exactly; press Generate to apply the new composition algorithm.

## Earlier Ableton observations (1.0 development build)

- Live's scanner discovers Undertow, and Live loads it as a VST3 instrument with MIDI output.
- A second MIDI track can select the Undertow track and its plug-in MIDI output, with Operator supplying the sound.
- The saved validation recording contains 42 intended note events matching the saved generator pattern's pitches, velocities, and repeated positions. Nontruncated durations differ by no more than approximately one sample at the 48 kHz test rate; the final note ends at recording stop.
- That recording also contains three unexpected sustained notes (MIDI pitches 2, 70, and 105, all velocity 127) spanning its full recording duration. They are absent from the generator's saved pattern, and pitches 2 and 105 are outside its generation range. Their origin has **not been established**. The subsequent 1.1 removal of the MIDI-input bus prevented Live from loading the instrument. The 1.2 compatibility repair restores the required bus while discarding input events. The origin of the earlier sustained notes and a clean repeat recording still need verification.
- The actual Max device loads immediately before Operator and displays its native interface, piano roll, and generated notes. Two instances produced different patterns on initialization. Saving, reopening the validation Set, and saving again preserved the entire serialized Max state exactly, including its nine-note Funk pattern and settings.
- Updated Max action buttons are exposed through Live's accessibility interface. Generate changed the seed from 1010887972 to 3665323741 and produced a different phrase. Create Clip created `Undertow · C MINOR · FUNK` in scene 1 on the containing Operator track and switched generation off. The saved Live clip has all seven expected notes, exact pitches/velocities, onset/duration differences under 0.000000105 beats, and the expected four-beat loop.
- The native VST editor snapshot was visually inspected for layout and clipping. The actual Max device interface was also inspected in Live.

## Remaining host checks

The following are pending: loading the repaired 1.2 VST3 in Live, the final Max disclosure/default visual checks, fresh Live playback/recording of the final 1.2 build; the complete native MIDI drag gesture into Live; Max Save MIDI through the actual host; and a sustained listening/production session using different instruments. File generation, parsing, state serialization, and mocked clip operations already have automated coverage. The host observations above used a dedicated validation Set. Attempts without a verified result are not counted as successful checks.

## Scope and limits

- This binary is Apple Silicon macOS only. No Intel, Windows, AU, or other-host binary validation is claimed.
- Ordinary VST3 uses Ableton's documented two-track routing. The Max MIDI effect provides placement before an instrument on the same track.
- Patterns use 4/4 and 1, 2, 4, or 8 bars. The engine does not infer chords or keys from audio. Published research informs constraints; the numerical style probabilities are authored design choices.
- The VST schedules MIDI on the audio callback. The Max edition runs on Max's message thread and can jitter under load; use its exported MIDI clips for host-scheduled timing.
- Both editions generate MIDI only. Glide, accent response, envelope, and tone depend on the destination instrument.
- Shortening a loop drops locked notes that no longer fit. Preserved locks otherwise take precedence over new pitch settings.
- The Max device needs its three adjacent JavaScript files. Keep the folder together. Its accessible, nonstored action controls require Max 9.1.3 or later; the tested bundled runtime is 9.1.5.
- General VST parameter automation for musical controls is not implemented; settings and exact notes are saved in plug-in state. Max exposes its native controls.
- Licensing information and dependency notices are in `THIRD_PARTY_NOTICES.md`.
