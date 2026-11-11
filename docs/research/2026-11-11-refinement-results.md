# Phrase refinement: implementation and evaluation, 11 November 2026

Baseline: `595c60e5d2b233338608be2ac01849df91449213`. This is a subsequent refinement, not the earlier hierarchical-motif rewrite. The baseline already repeated motifs and resolved connectors.

## What changed

- The initial core can now use both third and fifth, and Deep can realize its enabled upper tonic. A backward-weighted finite-state sampler considers feasible continuations, a chosen endpoint, the loop return and a bounded third/fifth budget before drawing each pitch. The weights are authored; there is no trained artist model or stored bassline bank.
- Deep and Funk can delay their first attack or anticipate a structural beat while omitting that beat. Optional answers depend on selected arrivals.
- Rolling varies its generated two-beat cell and repeats its actual pitch, gate and accent pattern, with a held-note role among detached attacks.
- Default unornamented Offbeat now has all four between-kick pulses. Density can thin them or attach pickups.
- Acid selects accent and held-note roles separately. Disco upper-tonic replies use shorter gates than lower support notes.
- Exact locks, C1/one-bar defaults, MIDI-only output and existing project-state formats remain compatible. The shared web core mirrors Max; approved media is unchanged.

Research rationale and limits are in [MUSIC_THEORY](../MUSIC_THEORY.md), the [sequence-model evidence](2026-11-11-sequence-model-evidence.md) and [style evidence](2026-11-11-style-refinement-evidence.md). Pachet et al.'s finite-length constrained sampling supplies the algorithmic principle; the cited groove studies and original lessons motivate rhythmic relationships and articulation. None supplies our exact probabilities or proves listening quality.

## Fixed comparison

The baseline and pitch-only intermediate were each captured at one and two bars before the next implementation stage. Final audits cover one, two and eight bars. Every corpus uses 128 dispersed seeds, six styles, default and maximum Melody profiles, and both C++ and Max: **3,072 records per corpus, 9,216 final records**. Each record also includes a bounded Variation result. The one-bar audit now reports unavailable two-bar statistics as null rather than inventing a comparison.

Controls: C natural minor, base MIDI36, Density .55, Swing .12, Gate .65, Melody .35 or 1, Start on root on, Root notes only off, Octave jumps on. Seed `i` is `(0xa341316c + i * 2654435761) mod 2^32`.

Mean absolute last-to-first pitch interval, default one-bar C++ patterns, in semitones:

| Style | Baseline | Revised |
| --- | ---: | ---: |
| Deep | 1.56 | 1.12 |
| Rolling | 3.77 | 0.66 |
| Offbeat | 1.70 | 0.48 |
| Acid | 1.73 | 0.83 |
| Disco | 10.03 | 5.73 |
| Funk | 2.26 | 1.06 |

This is a description, not a quality score: an octave return can be intentional, especially in Disco. All final audit conditions had zero scale violations, unresolved nontriadic connectors, long nontriadic notes and compound leaps. The baseline already satisfied those constraints; maintaining them is not evidence of aesthetic improvement.

## Verification

- Native development build and all four CTest groups passed: engine, processor/state, editor and actual VST3 wrapper.
- 669,370 native engine assertions; independently rerun with AddressSanitizer and UndefinedBehaviorSanitizer.
- 38 Max integration/engine checks and 7 audit-metric tests passed.
- 1,260 cross-edition patterns across 180 scenarios matched pitches, note counts, velocities, seeds and locks exactly; timing tolerance was 1e-12 beat. Scenarios include repeated Variation, changed settings and extreme saved locks.
- An independent read-only review instrumented 36,864 extreme settings: no zero/nonfinite continuation mass or unspent pitch budget. 4,608 gate comparisons retained timing/pitch/velocity and monotonic durations; 16,128 unornamented density cases retained nested onset skeletons. No actionable review findings.
- A local optimized C++ probe generated 12,000 valid one/eight-bar phrases in about 3.50 microseconds each on average. This is machine-specific generator time, not end-to-end DAW latency or a worst-case deadline guarantee.

## Listening artifacts and remaining limits

Local development artifacts are in `dist/refinement-2026-11-11/` (git-ignored): a listening guide, all corpora/source snapshots, exact MIDI, six neutral before/after comparisons and actual Serum Acid comparisons. Every comparison uses the first three default corpus seeds, each loop repeated twice at 126 BPM. No attractive examples were selected after rendering. The order is openly labeled before/revised, so this is not a blinded preference experiment.

Acid uses the user's `TSP_S290V3_Bass_acid_sweep.SerumPreset` in Serum 2.1.4, with a +24-semitone audition-only transformation. Six isolated renders verified processor/controller state, preset hash and exact transposed MIDI events. One shared gain provides headroom; raw float renders remain available. Original C1 MIDI, website picks and approvals are unchanged. Other styles use identical original synthetic accompaniment and pluck before/after.

Perceived improvement and artist resemblance remain unapproved. The device assumes a tonic vamp; key/mode alone cannot describe an arbitrary backing chord progression. MIDI velocity/gates are not a TB-303 slide circuit. Native checks do not substitute for a fresh reload and listening test in the user's Live session. Public release packaging stays on hold.

To reproduce the source audits, run `python3 plugin/scripts/audit_generator.py --output <fresh-directory> --seeds 128 --bars 1` (also 2 or 8) at the respective source revision. `plugin/scripts/render_auditions.py` renders the neutral comparisons from two matched corpus directories. The private Serum preset and local audit/audio artifacts are not redistributed with the public source.
