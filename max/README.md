# SUBMOTIF by Habeeb for Max for Live

This is the **single-track edition**: place `Submotif.amxd` before any instrument on an Ableton MIDI track. The instrument supplies all sound. Live Suite, or Live Standard with Max for Live, is required, with **Max 9.1.3 or later** for the accessible action controls. The tested bundled Max version is 9.1.5.

## Install

Keep these four files together in one folder:

- `Submotif.amxd`
- `undertow.js`
- `undertow-core.js`
- `undertow-roll.js`

Copy that folder to your Ableton User Library, or add the folder to Places. Drag `Submotif.amxd` onto a MIDI track **before** the instrument. The device is intentionally editable and ships with adjacent source files; moving the AMXD alone will lose its dependencies. `Submotif.maxpat` is an additional source copy and is not the device to load in Live.

## Make a bassline

Choose **Style**, **Key**, **Scale**, and **Length**, then press **Generate**. New devices start in C minor at C1 with a one-bar loop. **Density** chooses fewer or more notes; **Melody** chooses less or more pitch movement. Each Generate constructs a fresh motif from weighted choices, then develops it through repetition and related answers. There is no premade bassline bank.

**Change a few notes** keeps the current rhythm and most notes while making a small edit to the phrase. It becomes unavailable while settings are pending: press Generate to apply those settings first. Click any note in the piano roll to lock it; locked notes turn amber and survive regeneration. **Unlock all** appears when notes are locked. Undo/Redo retain up to 32 local editing steps.

**More controls** reveals Swing, Note length, Register, and the pitch switches. The compact device stays 169 pixels tall and expands from 1000 to 1260 pixels wide. **Fewer controls** hides those extras without changing their values, the phrase, locks, or history. This disclosure is a view preference for the current device instance, separate from its saved musical state.

Turn **Live MIDI on** and start Live's transport to hear the pattern through the next instrument. While the generator is on it replaces incoming track MIDI. **Live MIDI off** stops generated notes and lets incoming clips, controllers, and keyboard MIDI pass through. **Stop notes**, under More controls, releases held notes. Stop, seek, loop, output-state changes, and pattern replacement also release generated notes.

The generator uses four quarter-note beats per bar. It follows the global transport and tempo; its phase starts at the beginning of the song timeline rather than at a launched Session clip. The pattern is monophonic and exports without overlapping notes. Instrument glide, accent response, and timbre depend on the destination instrument.

**Root notes only** limits new notes to the tonic pitch class. **Octave jumps** permits the same pitch class in another octave. **Start on root** makes the first sounded, unlocked note the base tonic without forcing its timing onto the downbeat. Locked notes take precedence over newly selected settings. Register uses Live's octave convention: Key C with Register 1 is C1, MIDI note 36.

## Put the notes in Live

1. Select a scene in Session View whose slot is empty **on the track containing Submotif**.
2. Press **Create Clip**.
3. Submotif creates a MIDI clip in that slot and switches its generator off. Launch the clip to hear it, or drag it to Arrangement View.

It does not launch clips automatically, create tracks, replace existing clips, or write into another selected track. For a device nested inside a rack, it resolves the containing track through the device's canonical parents. If the target slot is occupied, choose an empty scene and try again.

**Save MIDI** writes a standard `.mid` file with the exact displayed notes and loop length. Drag that file from Finder into Live or another DAW. The VST3 edition supplies direct operating-system drag-out; this native Max edition uses Create Clip or Save MIDI.

Settings, exact notes, note locks, and the output switch are stored with the Live Set through a `pattr` state parameter. Save the Live Set to keep them. Parameter changes apply to the next **Generate**; they do not silently change a displayed pattern or its export. Existing Live Sets retain their saved key, register, and length. The new defaults only affect a new device instance.

## Implementation and checks

`build_device.py` produces the actual MIDI-effect AMXD and its readable patch source. It uses the same `ampf` / `mmmm` / `ptch` envelope as the installed Max MIDI Effect template. JavaScript uses the legacy ES5 engine and requires no Node package or network access in Live.

Playback reads the host-linked Max transport's raw ticks every 2 ms, with note cleanup on discontinuities. JavaScript is serviced on Max's low-priority message thread: this edition is **not sample accurate**, and heavy UI activity can increase timing jitter. For timing-critical playback, create a MIDI clip so Live schedules the notes, or use the VST3 edition's audio-thread scheduler.

Run the automated checks from the repository root:

```sh
python3 max/build_device.py
node tests/max-tests.js
```

The suite checks musical constraints, procedural diversity, seed reproducibility, locks, MIDI file bytes/length, scheduler note cleanup, adapter state round-trip, safe clip targeting, pending-setting behavior, disclosure without state changes, presentation bounds and control overlap, and AMXD structure. It also compares the embedded AMXD patch with the readable source. Tests with a mocked Max/Live API do not substitute for loading and exercising the device in Ableton.

The UI keeps the existing internal parameter names for saved sets and automation: Melody remains `Movement`, Note length remains `Gate`, and Register remains `Octave`. Action controls retain Max 9.1.3's Visible (Not Stored) mode so loading a project cannot invoke Generate, clip creation, or export.

Implementation references: [device width with live.thisdevice](https://docs.cycling74.com/reference/live.thisdevice/), [native sliders](https://docs.cycling74.com/reference/live.slider/), [native action controls and inactive state](https://docs.cycling74.com/reference/live.text/), [Max transport](https://docs.cycling74.com/reference/transport/), [JavaScript LiveAPI](https://docs.cycling74.com/apiref/js/liveapi/), [ClipSlot create_clip](https://docs.cycling74.com/apiref/lom/clipslot/), [Clip add_new_notes](https://docs.cycling74.com/apiref/lom/clip/), and [saving state with pattr](https://docs.cycling74.com/userguide/pattr/). Musical sources and the distinction between published findings and our authored style rules are in [MUSIC_THEORY.md](../docs/MUSIC_THEORY.md).
