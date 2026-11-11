# Engine audit — November 11, 2026

## Finding

The current generators are valid MIDI writers, but weak phrase composers. Their probabilistic constraints operate primarily at individual onsets. They do not construct a recurring rhythmic/pitch idea, know the track's chords, assign ornamental notes a target, or deliberately connect the loop ending to its beginning. The user's report of excessive randomness is consistent with the structure observed below. This audit does **not** claim to measure enjoyment, assess the user's synth sound, or prove that every nonchord tone is a clash.

No product generation code was edited for this audit. Release packaging remains on hold.

## Reproducible baseline

Run from the project root:

```sh
python3 scripts/audit_generator.py --output docs/research/engine-after-NEW-DATE
```

The helper compiles an isolated adapter against the actual C++ engine and loads the actual ES5 Max engine. It does not launch a DAW. The captured baseline is in [`engine-baseline-2026-11-11/`](engine-baseline-2026-11-11/):

- `manifest.json`: exact settings, seed formula, metric definitions, source/corpus SHA-256 hashes.
- `corpus.jsonl.gz`: all original and one-step Variation note sequences.
- `patterns.csv`: per-pattern observations; `summary.csv`: means, population standard deviations, and eligible-pattern counts.
- `examples/`: 48 original MIDI/JSON examples, chosen by fixed seed index 0 and 63 before inspecting results. These are audit fixtures, not product presets.
- `source-snapshot/`: the exact old C++ header/implementation and Max core, retained solely for offline before/after comparisons.

There are **6,144 base phrases**: 256 deterministic seeds × six styles × two Movement levels × two engines. Each has two bars, C natural minor, C1 base/MIDI36, Density55%, Swing12%, Gate65%, Root Only off, Root Anchor on, Octave Jumps on. Movement is either default35% or100%. Seed `i` is `(0xa341316c + i * 2654435761) mod 2^32`. Scattered 32-bit seeds avoid overrepresenting the correlated initial draws produced by tiny sequential xorshift seeds. A same-settings Variation uses `(seed + 2654435769) mod 2^32`.

All **6,144 phrases have zero out-of-scale notes**. This verifies a narrow technical property. Scale membership alone cannot establish harmonic fit against the user's unknown backing chords.

Metrics are descriptive. Rhythm similarity is the Jaccard intersection/union of the two bars' onset positions at960PPQ, with1 meaning identical sets. A compound leap exceeds12semitones. Variation ratio is note count after one Variation divided by original count. The tables are means across256phrases, not target values inferred from a paper.

## Baseline by style

| Engine / style | Bar rhythm similarity, default → high Movement | Compound leaps %, default → high | Variation count ratio, default → high |
| --- | ---: | ---: | ---: |
| CPP / Deep | 0.31 → 0.32 | 2.9 → 9.8 | 1.43× → 1.44× |
| CPP / Rolling | 0.32 → 0.34 | 2.4 → 8.7 | 1.42× → 1.42× |
| CPP / Offbeat | 0.50 → 0.50 | 3.2 → 9.3 | 1.32× → 1.33× |
| CPP / Acid | 0.31 → 0.31 | 3.2 → 9.4 | 1.44× → 1.44× |
| CPP / Disco | 0.41 → 0.43 | 11.2 → 21.3 | 1.38× → 1.36× |
| CPP / Funk | 0.29 → 0.30 | 5.2 → 12.2 | 1.46× → 1.45× |
| MAX / Deep | 0.26 → 0.25 | 2.6 → 7.8 | 1.38× → 1.35× |
| MAX / Rolling | 0.34 → 0.33 | 2.2 → 7.2 | 1.31× → 1.29× |
| MAX / Offbeat | 0.67 → 0.66 | 2.3 → 7.4 | 1.17× → 1.16× |
| MAX / Acid | 0.35 → 0.33 | 2.0 → 7.0 | 1.28× → 1.29× |
| MAX / Disco | 0.40 → 0.42 | 5.9 → 19.6 | 1.26× → 1.27× |
| MAX / Funk | 0.28 → 0.28 | 4.7 → 13.8 | 1.35× → 1.37× |

Exact repeated bar rhythms are absent in the256default-seed examples for Deep, Rolling, Acid and Funk in both editions. C++ Disco repeats its bar rhythm in0.39% of default cases; Max Disco in0%. Offbeat is the exception because its allowed rhythmic vocabulary is much narrower: C++ repeats in4.69%, Max in18.75%. Identical onset-and-pitch bars are rarer still. These figures do not mean every bassline must repeat exactly; they show that no explicit repetition policy is currently doing the work.

## Source-level causes and observable consequences

1. **No motif is generated or remembered.** In C++ `generate`, the loop visits every sixteenth of every bar, calling `onsetWeight` and then drawing a fresh coin toss. Max does the same in its `generate`. Local `previousStep`/gap/run state changes probabilities but cannot reproduce an earlier gesture. There is no phrase-level statement/answer representation. Merely raising a quarter/eighth weight makes those positions more common across a population; it does not make the second bar remember the first.

2. **Harmonic stability is a probability, not a role.** C++ `selectPitch` chooses a tonic or a weighted/neighbor scale degree without knowing whether the onset is a structural attack, a weak ornament, or the phrase's resolution. Max uses the same broad scale constraint with a different weighting scheme. Root Anchor only fixes the first sounded note of the entire phrase. Neither implementation infers or receives a chord progression. C/Eb/G is therefore used below only as an explicitly assumed tonic-minor reference sonority.

| Engine / style | Quarter-beat notes outside tonic triad %, default → high Movement |
| --- | ---: |
| CPP / Deep | 12.1 → 25.5 |
| CPP / Rolling | 13.2 → 27.2 |
| CPP / Offbeat | 2.7 → 28.9 |
| CPP / Acid | 13.1 → 22.8 |
| CPP / Disco | 11.3 → 24.2 |
| CPP / Funk | 10.9 → 25.0 |
| MAX / Deep | 10.8 → 28.7 |
| MAX / Rolling | 7.5 → 25.9 |
| MAX / Offbeat | n/a → n/a |
| MAX / Acid | 8.2 → 26.2 |
| MAX / Disco | 9.3 → 28.7 |
| MAX / Funk | 7.4 → 26.6 |

These means include only phrases with at least one quarter-beat onset; counts are in the CSV. Max Offbeat has none, hence n/a. C++ Offbeat has few such phrases, so that row must not be read as a rate over all Offbeat notes. D/F/Ab/Bb are legitimate scale notes; placing one on an exposed strong attack without preparation or resolution may nevertheless conflict with a sustained C-minor backing sonority. A different backing chord changes the assessment.

3. **“Movement” is not voice-leading.** C++ frequently walks one scale degree from an internal `lastDegree`, but octave selection happens afterward and is not part of that state. Max measures proximity to a prior sounding pitch but can then add an independent octave displacement. Neither requires a nontriadic neighbor to return or an approach to lead into a specific next target. More Movement therefore increases unrelated pitch vocabulary and register disturbance, rather than producing controlled development.

4. **Octaves are independently drawn for almost every selected note.** C++ Disco adds0.62 to octave probability at offbeat eighths, then can transpose a nonroot degree by12. At high Movement, its average pitch span is17.93semitones and21.34% of cyclic adjacent intervals exceed an octave. Max Disco's span is19.93semitones, with19.60% compound leaps. An octave gesture is musically useful; independently lifting arbitrary degrees can produce awkward compound leaps and a poorly connected loop seam.

5. **Articulation is calculated before its musical role is known.** C++ selects an arbitrary duration from a range, then shortens it at the next onset. That preserves monophony but often uses all the available space. A clipped duration is a safety correction, not a deliberate bass articulation. Max uses a few duration categories but still draws them independently of a recurring motif. Neither preserves a repeated accent/gate gesture when the next bar is generated.

6. **Variation is an overlay and tends to fill the gaps.** C++ retains about76% of eligible old notes, Max about62%, then constructs another complete candidate phrase and merges it. One default C++ Variation raises note count by32–46% on average across styles; Max by17–38%. That is a material change in density even though the Density control has not moved. The current safety cap prevents unbounded capacity, but does not make the result a restrained variation.

7. **The two editions are musically different products.** The C++ and ES5 implementations have different onset probabilities, pitch distributions, octave schemes and duration rules. For example, high-Movement tonic-note share in Rolling is53.37% in C++ versus37.15% in Max. Reproducing a musical fix independently in both would continue that divergence; a shared composition design and cross-edition fixed-seed parity are needed.

## Concrete, non-selected reproduction

The fixed example seed2464687099 is index63 from the sampling rule, chosen before inspection. In C++ Rolling/default, the first bar starts with C1 at0,0.265and0.5, then leaves a large gap before2.5; the second bar instead begins on Eb2/MIDI51 at4.0. In C++ Disco/high-Movement, the final note is Ab2/MIDI56 atbeat7.0, followed by a repeated-loop C1/MIDI36: a20-semitone seam leap. Both examples are scale-correct. Their MIDI and full notes are under `examples/` and should be auditioned with the same sound and tempo before/after changes.

## Research-to-design interpretation

The following sources support musical principles, not numerical acceptance thresholds or a guarantee of taste:

- Berklee instructors Anthony Vitti and Lenny Stallworth demonstrate a repeated A idea with related B/C answers in an A–B–A–C groove structure. That directly highlights the absence of motif and phrase memory here. [Berklee: Q&A bass groove construction](https://online.berklee.edu/takenote/bass-groove-basics-build-better-lines-with-qa-technique/).
- StudyBass distinguishes chord tones from other scale tones and teaches scale tones as short connecting material. This motivates structural targets plus prepared, brief resolving notes, instead of treating every scale degree as equally eligible at any onset. [Scale tones in basslines](https://www.studybass.com/lessons/harmony/scale-tones-in-basslines/) and [chord tones](https://www.studybass.com/lessons/harmony/chord-tones-in-basslines/).
- TalkingBass teaches Disco octaves as a repeated rhythmic figure. This supports planned octave roles rather than independent register flips. [Disco bass octaves](https://www.talkingbass.net/disco-bass-octaves/).
- Butler's EDM analysis and Witek's groove studies, cited in [`../MUSIC_THEORY.md`](../MUSIC_THEORY.md), support attention to metrical layers, repetition and rhythmic space. A drum-pattern syncopation finding is not a validated bassline-quality function.

The supported redesign hypothesis is hierarchical: freshly construct a rhythmic identity and harmonic target plan, repeat/develop it, attach limited resolving ornaments, then apply articulation and safety. It remains procedural on every Generate; reusing a motif inside a phrase is not selecting a stored bassline.

## Evaluation limits and next comparison

Do not optimize every metric upward. Perfect repetition can be dull; octave gestures can be stylistically right; nontriad tones can be expressive. No score in this report measures musical quality. The intended checks are narrower: recognize an opening, retain a stable pulse/rest pattern, give ornamental tones a target, avoid accidental compound register jumps, and keep Variation from uncontrolled densification.

Use the identical fixed seeds/settings after revision, preserve both data sets, and provide equal-tempo/equal-timbre MIDI/audio auditions across all six styles. The user must be able to judge the actual grooves. Also verify backing key/chords and avoid simultaneous playback of a generated line and an exported clip during listening; this audit did not inspect the user's arrangement or audio routing.
