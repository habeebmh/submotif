# Submotif website

This folder contains the static landing page, browser generator, approved recordings, legal pages, and audition gallery. It requires no package installation or build step.

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

Firebase configuration stays at the repository root and serves this folder. Hidden development files, screenshots, scripts, and Markdown are excluded from deployment. See the [website guide](../docs/WEBSITE.md) and [hosting instructions](../docs/WEBSITE_LAUNCH.md). Downloads remain disabled while release packaging is on hold.
