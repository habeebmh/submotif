# Submotif landing page

A dependency-free static page, with original product UI and a browser preview driven by the exact portable Submotif engine. All CSS, JavaScript, fonts, and artwork are hosted locally. No analytics, cookies, or third-party embedded players are used.

## Preview

Serve the `web/` folder over HTTP. For example, from the project root:

```sh
python3 -m http.server 8080 --directory web
```

Open `http://localhost:8080`. ES modules require an HTTP server; double-clicking the file will not provide the complete interactive preview.

## Publication settings

This is the published Submotif preview, named under the owner's delegated naming request. Source is public at `https://github.com/habeebmh/submotif`. Firebase Hosting deployment completed on November 11, 2026 at `https://submotif.web.app`. The intended domain is `submotif.com`; it has not been purchased, and availability can change. The live Firebase Hosting origin is `https://submotif.web.app`; canonical and absolute social-image metadata use it. A custom domain is optional: the assigned `web.app` origin can host the preview while a custom domain is purchased and attached later. Public downloads remain on hold. The six-example showcase is enabled with the owner-approved Serum Acid recording two octaves higher. Do not present unresolved release or showcase status as completed.

`robots.txt` disallows all crawling and `index.html` includes `noindex,nofollow`. Remove the blanket preview restriction only when publication is approved; keep `/review/` out of indexing. A custom `404.html` is included.

Paths below are relative to `web/` unless a repository folder is named.

Edit `assets/site-config.js`:

- `repository`: canonical GitHub repository URL.
- `releaseUrl`: permanent GitHub `releases/latest` URL.
- `releaseReady`: keep false until a real downloadable release exists and packaging is authorized. When false, the page shows a preparation notice and disables the final download.
- `instagram`: the owner's confirmed HTTPS profile URL. The owner supplied the current profile. Blank means Instagram links and support copy stay hidden.
- `canonical`: actual assigned Firebase `web.app` origin initially, or the confirmed custom domain when attached; also set canonical and absolute social-image metadata in `index.html` before publication.
- `licenseStatus`: `AGPL-3.0-only`, explicitly selected by the owner. The project is free and open source; AGPLv3 permits forks and commercial distribution subject to its terms. The page does not claim that resale or derivatives are prohibited.
- `featured`: six explicitly approved showcase selections: `{style, audio, id, approved:true, pattern}`. The UI validates each MIDI pattern, requires same-origin audio, and shows a separate showcase only with all six unique styles explicitly approved. `showcaseBatchFingerprint` must match the exact 64-character SHA-256 preview-batch fingerprint in the owner-approved selection export and manifest. Each ID and style must match `showcaseManifest` (default `review/manifest.json`); the complete pattern must match exactly, or have a recorded owner-approved octave transformation that changes only pitches. The transformation must link the original source MIDI hash, and its rendered/exported native MIDI hash must agree. The current Acid example is explicitly approved at +24 semitones; the manifest must identify the C natural minor, two-bar, 126 BPM review batch. `showcaseApprovalManifest` (default `assets/audio/approved-manifest.json`) separately approves each final recording path and links its source MIDI hash to the original candidate. Authorized Serum renders must name the chosen preset and have approved status; no neutral acid recording is substituted while that render is pending. The owner-approved config is separate from the original candidate data. Each recording has an export of its exact pattern. Showcase playback pauses the live preview and other recordings; live playback pauses the showcase. Generate always runs the algorithm anew.

The live preview's synth is specific to the webpage; Submotif itself remains MIDI-only. Preview volume is limited and audio begins only after a deliberate Play click. Playback stops after four repeats, on another generation, on tab hiding, or on page exit.

## Audition approval

`review/` contains the separate 60-candidate review flow and its own manifest. The owner selected deep-01, rolling-06, offbeat-01, acid-03, disco-01, and funk-02. Five original recordings are approved. The owner also approved Acid at +24 semitones with `TSP_S290V3_Bass_acid_sweep.SerumPreset`; the native MIDI download and displayed pattern use that exact higher register. `selected-examples.html` provides the six chosen patterns and both mix/bass-only recordings, with recorded approval status visible; it does not grant or persist approval. The main showcase is visible because all six now have complete approval and provenance. Keep the review files out of a public release if the owner wants this approval process private.

## Assets and licensing

- `assets/undertow-core.js` is copied without changes from `plugin/max/undertow-core.js`; re-copy after engine changes.
- `assets/images/submotif-editor.png` and `submotif-editor-large.png` are actual native v1.2.2 Submotif captures, copied from `dist/ui-v1.2.2/`.
- `share.jpg` is a rendered capture of the landing page hero.
- Runtime layout/motion components are from `design-taste-frontend` 2.5.1 under `assets/LICENSE-taste-skill.txt`.
- Familjen Grotesk and IBM Plex Mono are self-hosted with their OFL notices under `assets/fonts/`. Host Grotesk remains available for auxiliary pages.
- The waveform brand graphic carries forward the project’s existing original mark.

Design decisions, recorded deals, facts and the finish-gate report are under `.taste-skill/`. Screenshot and audit output is under `shots/`; neither directory should be deployed.

## November 11 redesign

The earlier page shipped with an incomplete finish gate, accumulated mobile overrides, a decorative overlay, and insufficient checks of short phone viewports and live motion. This was an implementation and verification failure. The archived baseline under `.taste-skill/previous-build/` and short-screen report reproduce the excessive section heights.

The replacement uses the native editor as the hero image, one live generator window, one player with six explicit recording choices, and two plain Ableton routing diagrams. The extra repository/FAQ screen was removed; contribution and Instagram support links are in the footer. It keeps normal document scrolling and uses the upstream hero entrance, hover layer, and word-highlight moment. Reduced-motion behavior comes from the same upstream runtime. There is no timed recording rotation or autoplay. Approved MIDI, audio, provenance and the portable generation engine remain unchanged.

The redesign is a new requested build, with the previous failed gate retained separately. Current validation and viewport evidence are recorded under `web/.taste-skill/` and `web/shots/`, excluded from hosting.
