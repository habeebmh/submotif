# Submotif website

This folder contains the static landing page, approved recordings, legal pages, and audition gallery. It requires no package installation or build step.

From the repository root:

```sh
python3 -m http.server 8080 --bind 127.0.0.1 --directory web
```

Open `http://127.0.0.1:8080/`. The original audition gallery is `/review/`; the selected recordings are `/selected-examples.html`.

- `assets/`: self-hosted UI, fonts, graphics, audio, and the browser copy of the portable engine.
- `legal/`: rendered legal pages and exact license/reference copies.
- `review/`: the original audition batch, retained without changing its approval fingerprint.
- `scripts/`: tools for rendering legal pages and generating a future audition batch. These tools are excluded from Hosting.

After editing legal documents in `docs/legal/`, run `python3 web/scripts/build-legal-pages.py`. After changing the Max core, synchronize `plugin/max/undertow-core.js` to `web/assets/undertow-core.js` and run the cross-edition checks.

Firebase configuration stays at the repository root and serves this folder. Hidden development files, screenshots, scripts, and Markdown are excluded from deployment. See the [website guide](../docs/WEBSITE.md) and [hosting instructions](../docs/WEBSITE_LAUNCH.md). Download links point to the latest GitHub release; platform availability must match its published assets.


## Data graphics

The landing page uses the native plugin screenshot and diagrams of the approved MIDI. The six waveform graphics are derived from the approved recordings, not decorative wave shapes. To rebuild the SVG assets, install Python 3 and FFmpeg, then run from the repository root:

```sh
python3 web/scripts/build-visuals.py
```

The script checks the approval batch and recording hashes, writes the note/waveform SVGs and their provenance to `assets/graphics/`, and leaves the audio and MIDI untouched. Commit the generated assets so hosting needs no build tools.

`showcase-visual-probe.js` exercises all six selectors and verifies their audio, MIDI and graphic mapping when passed to the design-taste browser checker's `--probe` option. `scroll-performance-probe.js` checks the scroll path and confirms that no canvas renderer has returned. Test desktop, tablet and phone layouts after UI changes; respect reduced motion and preserve the approved Acid transposition.
