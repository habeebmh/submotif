# Musical phrasing revision: measured changes

This is a structural comparison, not a musical-quality score. Listening examples are in [`../../dist/auditions-v1.1/`](../../dist/auditions-v1.1). Each comparison alternates old and revised for three predetermined seeds, with identical bass synthesis, drums, tempo and one shared gain.

## Matched two-bar comparison

256 seeds × six styles × two Movement settings × two editions = **6,144 base phrases per revision**, each with one same-settings Variation. Key C natural minor, base MIDI36, Density55%, Swing12%, Gate65%; Movement35% or100%. The following means use C++ at Movement35%. Both revised engines share composition rules and are also compared directly by the parity test.

| Style | Bar rhythm similarity old → revised | Variation note-count ratio old → revised | Exact full-note retention at shared bar onsets old → revised |
|---|---:|---:|---:|
| Deep | 0.31 → 0.96 | 1.43× → 1.00× | 0.2% → 83.6% |
| Rolling | 0.32 → 0.97 | 1.42× → 1.00× | 0.8% → 88.6% |
| Offbeat | 0.50 → 1.00 | 1.32× → 1.00× | 0.0% → 72.2% |
| Acid | 0.31 → 0.97 | 1.44× → 1.00× | 1.0% → 88.7% |
| Disco | 0.41 → 1.00 | 1.38× → 1.00× | 0.0% → 88.4% |
| Funk | 0.29 → 0.96 | 1.46× → 1.00× | 0.5% → 85.7% |

Rhythm similarity is first/second-bar onset intersection divided by union. Full-note retention compares pitch, duration and velocity only at shared onsets; read it together with onset similarity. Higher values are not inherently better: the change demonstrates deliberate repetition rather than proving enjoyment.

Across the revised two-bar corpus: zero out-of-scale notes, zero compound leaps (>12 semitones), zero unresolved nontriadic notes under the audit’s explicit tonic-triad/short-step criterion, and a Variation count ratio of 1.0 in every condition. The legacy engine was already scale-correct; scale membership was not the diagnosed problem.

## Diversity and longer form

| Style, default Movement | Distinct first-bar pitch/onset motifs among 256 seeds | Distinct two-bar pitch/onset motifs | Pairwise two-bar collisions |
|---|---:|---:|---:|
| Deep | 227 | 246 | 0.03% |
| Rolling | 140 | 210 | 0.24% |
| Offbeat | 17 | 49 | 5.53% |
| Acid | 254 | 256 | 0.00% |
| Disco | 115 | 181 | 0.57% |
| Funk | 247 | 254 | 0.01% |

These signatures exclude duration, velocity and seed metadata. A fresh computation can repeat an earlier structure, particularly the deliberately narrow Offbeat pulse. No claim of universal uniqueness is made. These counts are descriptive, not an estimate of the total search space or proof of a bank-free implementation; the source constructs rhythms and pitches at runtime.

Separate four- and eight-bar audits each contain 1,536 phrases (64 seeds per condition), bringing revised base-phrase coverage to 9,216. Lag-two repetition measurements expose the return of the statement in longer forms. These audits are additional checks, not matched old/new comparisons because the recorded baseline has two bars.

## Artifacts and reproducibility

- [Baseline corpus and original source](engine-baseline-2026-11-11/manifest.json)
- [Revised two-bar corpus and source hashes](engine-after-2026-11-11/manifest.json)
- [Four-bar audit](engine-after-four-bars-2026-11-11/manifest.json)
- [Eight-bar audit](engine-after-eight-bars-2026-11-11/manifest.json)
- [Metric definitions, denominators and limits](AUDIT_METRICS.md)
- [Academic and style research](../MUSIC_THEORY.md)

The audit condition is a tonic C-minor vamp and cannot establish fit to unknown chord changes. Broader scale/key/register/control constraints are covered by engine tests. Exact locks can intentionally override newly selected harmonic/register settings. No reference recording, artist sample or MIDI pattern is used.

The audio is an offline composition demonstration, not a new built-in synth. The actual plug-in still sends MIDI to the user’s instrument. The examples have not been certified by a listening panel or the reference artists. Release packaging remains on hold.
