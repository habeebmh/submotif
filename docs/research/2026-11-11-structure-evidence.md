# Structure, repetition and tonal roles

This revision combines the [style/artist research](2026-11-11-house-acid.md), [Disco/Funk research](2026-11-11-disco-funk.md), and [measured engine diagnosis](2026-11-11-engine-audit.md). The following additional academic sources explain why the design separates phrase structure from individual note choice. They do not certify Undertow's outputs.

## Repetition as evidence, not a quality guarantee

Margulis and Simchy-Gross (2016), “Repetition Enhances the Musicality of Randomly Generated Tone Sequences,” *Music Perception* 33(4), 509–514, DOI [10.1525/MP.2016.33.4.509](https://doi.org/10.1525/MP.2016.33.4.509), tested prior exposure through looping. Previously repeated tone sequences received higher subsequent musicality ratings, but the effect was small. This supports investigating repetition; it does not show that copying a random bar guarantees a good bassline or determine an ideal repeat/variation ratio. The [authors' institutional abstract](https://collaborate.princeton.edu/en/publications/repetition-enhances-the-musicality-of-randomly-generated-tone-seq/) was consulted.

## Plan the phrase before decorating it

Bhandari and Colton (2024), [“Motifs, Phrases, and Beyond: The Modelling of Structure in Symbolic Music Generation”](https://arxiv.org/html/2403.07995v1), accepted at EvoMUSART 2024, reviews approaches to symbolic musical structure, including explicit high-level planning followed by content generation. We adopt that separation as a software design: generate a motif with harmonic/rhythmic roles, then develop it. This review does not supply a house-bass algorithm or measured probabilities, and our deterministic procedural method is not a trained model from the paper.

## Connection means a destination

Robert Hutchinson's academic textbook *Music Theory for the 21st-Century Classroom*, [Non-Chord Tones](https://musictheory.pugetsound.edu/mt21c/NonChordTonesIntroduction.html), [Passing Tones](https://musictheory.pugetsound.edu/mt21c/PassingTones.html), and [Neighbor Tones](https://musictheory.pugetsound.edu/mt21c/NeighborTones.html), distinguishes tones by how they are approached and left. A passing tone connects through stepwise motion; a neighbor departs and returns by step. Our construction therefore chooses targets before ornamental pitches. A note that merely happens to be in a scale is not automatically a passing tone. The implementation's conservative requirement for short, resolving nontriadic notes is a product choice for a tonic-vamp generator, not a universal law of funk, acid, modal music or popular harmony.

## What the experiments can establish

Fixed-seed audits can demonstrate greater motif retention, fewer compound register leaps, actual target resolution, and no density accumulation under Variation. Those properties address diagnosed weaknesses. They cannot demonstrate enjoyment, dancefloor effectiveness, faithful artist style, or compatibility with unspecified chord changes. Equal-tempo/equal-timbre before/after audio accompanies the revision so those judgments can be made by listening.

The user's artist references guide taste. No reference recording, sample, MIDI transcription, or artist-specific preset is stored by the generator. Every Generate still constructs its own rhythm and pitches with fresh weighted choices.
