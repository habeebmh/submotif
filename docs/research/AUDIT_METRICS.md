# Interpreting the generator audit

`scripts/audit_generator.py` reports descriptive measurements of generated MIDI. It does not calculate a musical-quality score. Run it with a fresh output folder, for example:

```sh
python3 scripts/audit_generator.py --output /tmp/undertow-audit --seeds 256
python3 scripts/audit_generator.py --output /tmp/undertow-audit-four-bars --seeds 32 --bars 4
```

`--bars` accepts 2, 4, or 8; the default remains 2. The existing recorded baseline is two bars. Compare like lengths, controls, and seed counts. Adapter settings are explicit, so a later change to product defaults cannot silently change the audit condition. Saved generator source and its hashes identify what produced each corpus.

## Repeated material

The new `repeat_lag1_*` fields compare every bar to its preceding bar. The `repeat_lag2_*` fields compare every bar to the bar two positions earlier, making an A/B/A/B pattern visible. A two-bar audit has no lag-two comparison and reports null for those fields.

- `onset_overlap_pct` is `200 × shared onsets / (onsets on both sides)`, summed across the compared pairs. Added and removed attacks reduce overlap. Onsets are rounded to 960 ticks per beat within each bar.
- `pitch_retention_pct`, `duration_retention_pct`, and `velocity_retention_pct` compare notes only where an onset is shared. They measure identical MIDI pitch, duration rounded to 960 ticks per beat, and integer velocity, respectively.
- `full_note_retention_pct` requires all three attributes to match at a shared onset. It is also conditional on shared onsets, not a whole-phrase equality percentage.

Always read retention with onset overlap. One matching note can give 100% retention while most of the phrase changes. No shared notes produces null, not 0% or 100%. Longer phrases pool note-pair counts within each pattern; condition summaries then average the per-pattern measurements, with eligible-pattern counts. Timing or velocity humanization can lower exact retention without making the phrase less musical.

Legacy fields `bar_onset_jaccard`, `exact_bar_rhythm_pct`, and `exact_bar_pitch_rhythm_pct` still compare only bars one and two, including in longer audits. `notes_per_bar` and `empty_bar_pct` cover the whole requested phrase.

## Diversity across seeds

Each condition receives counts of distinct first-bar and first-two-bar onset motifs, plus counts that include MIDI pitch. These are summary fields, not per-pattern measurements or averages. Signatures use the nearest sixteenth and exclude velocity, duration, and seed identifiers. Thus changing only articulation does not count as composing a different structural motif. Coarse timing reduces sensitivity to small timing deviations; it intentionally loses finer rhythmic detail.

`first_bar_pitch_onset_collision_pct` and its two-bar counterpart report the share of unordered pairs of different seed indices with identical signatures. For signatures A, A, B, one of three pairs collides: 33.33%. One seed has no pair and reports null. A shared first bar followed by different answers remains one first-bar motif but can produce several two-bar motifs.

These numbers describe the sampled outputs. They neither estimate the total possible output space nor prove originality, groove, or stylistic authenticity. Random noise can be very diverse; useful repeated offbeat or pedal figures can be less diverse. Evaluate both structure and listening results without treating maximum diversity or maximum repetition as a goal.

## Verification

`python3 tests/audit-tests.py` checks the metric semantics with small explicit fixtures. The extension was also checked against all 6,144 immutable baseline corpus rows: every pre-existing metric value remained unchanged. Separate 2/4/8-bar runs using the saved baseline C++ and Max sources produced the requested lengths; the default two-bar corpus matched the original records for the same tested seeds. Baseline artifacts were not rewritten.
