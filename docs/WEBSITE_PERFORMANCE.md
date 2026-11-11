# Website rendering regression

The November 11, 2026 report of black flashes exposed a gap in the earlier checks: page-load timings and screenshots did not test sustained rendering under contention.

## Cause

The hero opted into the design kit's animated `grain-gradient` WebGL effect. Its adaptive-resolution path calls `fit(false)` **after** `gl.drawArrays`. Changing `canvas.width` and `canvas.height` clears the drawing buffer, but that path does not redraw until the next animation callback. A slow frame can therefore cause the renderer to present a cleared, black buffer. Context replacement and phone background baking add work that this decorative surface does not need.

A browser probe reproduced three black buffers in 8.5 seconds with 35 ms of deliberate main-thread contention per frame at 1440 × 900. It sampled the buffer after the animation callback completed, so this is evidence of the specific clearing bug, not an inference from page-load timing. Normal unloaded scrolling did not reproduce the flash in that run.

## Fix

Use a static CSS background behind the native editor image. Remove the hero's `data-fx` opt-in, so the site never loads the shader module or allocates its canvas. Leave the upstream design kit untouched. Keep the hero entrance, hover feedback and event-driven word highlight. The generator, recordings and download hold are unaffected. Version the changed stylesheet URL and make the `/` response revalidate, matching the existing policy for `.html` URLs; the root previously advertised a five-minute cache lifetime.

This intentionally departs from the design skill's default of a shader effect on a software page. A decorative continuous renderer is unnecessary here and conflicts with the reported performance problem.

## Reproducing the check

`web/scripts/scroll-performance-probe.js` is an expression for the design skill's unmodified `consistency.mjs --probe` option. Run sequentially at desktop and phone widths. It measures a visible hero, repeated full-page scrolling, the same cases with the effect paused, and a constrained hero. It checks for black frames after canvas resets, hidden content, overflow, actual scroll travel and remaining effect resources. The constrained phase deliberately creates slow frames; its frame rate is not a performance target.

The fixed page must have no effect canvas or shader requests and no black-buffer events. This browser test is not a claim of universal frame rates on every device. A physical iPhone/Safari has not been tested.

Local Chrome results after the fix:

| Check | 1440 × 900 | 390 × 844 |
| --- | --- | --- |
| Effect canvases / shader requests | 0 / 0 | 0 / 0 |
| Black frames under contention | 0 | 0 |
| Full-page scroll distance reached | 3600 px | 3376 px |
| Hidden main content / horizontal overflow | None | None |

The old desktop hero submitted 240 WebGL draws over four seconds at rest; the fixed page submitted zero. Both fixed normal-scroll runs had a 95th-percentile animation-frame gap of approximately 16.7 ms. Reports are kept locally under `web/.taste-skill/scroll-repro.txt` and `scroll-fixed.txt` and excluded from hosting.
