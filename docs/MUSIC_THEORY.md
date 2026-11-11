# Submotif: musical design and research

Submotif constructs basslines from musical constraints and fresh random decisions. **Generate must not select a complete rhythm, melody, or bassline from a stored pattern bank.** Style choices change the probability rules that construct a line. A seed makes a result reproducible; a new generation uses a new seed. Repetition within a generated phrase is a compositional choice, not retrieval of a prerecorded or prewritten bassline.

This document separates published findings from our design decisions. The cited research does not validate Submotif, provide a universal definition of a good bassline, or prescribe numerical probabilities for the six styles. Listening in the user's track remains essential.

## Research that informs the design

**Syncopation and groove.** Witek, Clarke, Wallentin, Kringelbach, and Vuust (2014) found an inverted-U relationship between syncopation and ratings of pleasure and wanting to move in their tested drum-break stimuli: intermediate syncopation attracted higher ratings than the extremes. This motivates preserving a perceptible pulse while allowing displaced accents and rests. It does not imply that a fixed “50% syncopation” parameter is optimal, that note density equals syncopation, or that the result transfers unchanged to isolated basslines. DOI: [10.1371/journal.pone.0094446](https://doi.org/10.1371/journal.pone.0094446). [Full article](https://journals.plos.org/plosone/article?id=10.1371/journal.pone.0094446).

**Layered meter in electronic dance music.** Butler (2001) analyzes metrical ambiguity, dissonance, layering, and asymmetrical patterns in electronic dance music. A bass rhythm is heard alongside other layers, so a syncopated line can coexist with a clear kick pulse. We use metric positions and phrase context to shape probabilities rather than independently treating every subdivision as equivalent. Our implementation is an inference from this analysis, not Butler's algorithm. [Original article, *Music Theory Online* 7(6)](https://mtosmt.org/issues/mto.01.7.6/mto.01.7.6.butler.html).

**Rests are musical material.** Witek (2017, first published online 2016) analyzes groove as an interaction among rhythmic structure, bodily participation, and pleasure, including Helix's house track “Drum Track.” This motivates leaving useful rhythmic gaps. It is a theoretical account, not an experiment proving a specific bass/kick separation rule. We do not assume that a bass must always avoid the kick. DOI: [10.1111/musa.12082](https://doi.org/10.1111/musa.12082). [Article and abstract](https://onlinelibrary.wiley.com/doi/10.1111/musa.12082).

**Tonal stability.** Krumhansl and Kessler (1982) measured perceived tonal organization in Western major/minor contexts using probe-tone ratings. Their work supports distinguishing structurally stable tones from less stable ones. It motivates a tonal center and a preference for tonic/triadic support over uniform pitch choice. We do not use their ratings as bass-note probabilities, and the study does not establish identical hierarchies for Dorian, Phrygian, every culture, or every genre. DOI: [10.1037/0033-295X.89.4.334](https://doi.org/10.1037/0033-295X.89.4.334). [Original paper PDF](https://static1.1.sqspcdn.com/static/f/221758/23618809/1380667475290/KrumhanslKessler1982TracingPerceviedTonalOrg-SpatialRep-Keys.pdf).

**Melodic continuity.** Huron (2001) derives voice-leading practices from perceptual principles including pitch proximity. Our modest adaptation favors some repeated notes and nearby scale motion, while allowing deliberate larger intervals. This concerns continuity of a single bass part; Submotif does not solve multi-part counterpoint or infer a chord progression. Octave leaps are permitted stylistic gestures, not errors that must be eliminated. DOI: [10.1525/mp.2001.19.1.1](https://doi.org/10.1525/mp.2001.19.1.1). [Original paper PDF](https://www.musanim.com/articles/Huron.ToneAndVoice.2001.pdf).

**Constrained sequence generation.** Pachet, Roy, and Barbieri (2011) show how backward propagation of feasible continuation weights supports sampling finite sequences subject to constraints. Submotif applies this algorithmic idea to a small set of pitch roles, selected endpoints and a non-tonic attack budget. Its transition and contour weights are authored, not fitted to recordings or taken from the paper. The paper establishes an algorithm for a specified constraint class, not the musical quality of these rules or an empirical model of house bass. [Original IJCAI paper](https://www.francoispachet.fr/wp-content/uploads/2021/01/pachet-11b.pdf), DOI [10.5591/978-1-57735-516-8/IJCAI11-113](https://doi.org/10.5591/978-1-57735-516-8/IJCAI11-113).

**Relationships among rhythmic events.** Sioros and colleagues (2022) compared original, de-syncopated and algorithmically syncopated funk/rock excerpts. Original syncopation raised urge-to-move ratings relative to its removal; pseudorandom syncopation did not significantly improve them over the de-syncopated versions. Their analysis motivates attention to instrument roles, metrical placement and pickups, rather than a single syncopation score. The proposed causal explanations remain hypotheses, and their experiment does not validate our rhythm rules. [Original article](https://online.ucpress.edu/mp/article/39/5/503/182325/Syncopation-and-Groove-in-Polyphonic-MusicPatterns), DOI [10.1525/mp.2022.39.5.503](https://doi.org/10.1525/mp.2022.39.5.503).

**A direct house-bass experiment.** Duncan and Orgs (2024) manipulated bass syncopation and low-frequency amplitude in 20 newly composed house clips, rated by 179 listeners. The combination of higher syncopation and higher low-frequency amplitude produced the strongest urge to move. Only two syncopation levels were tested, with a fixed drum context; this does not establish an optimum, aesthetic superiority, or numerical style probabilities. It motivates evaluating bass in context with a controlled sound and level. [Original paper](https://discovery.ucl.ac.uk/id/eprint/10204599/1/Move%20your%20Body.pdf), DOI [10.1525/MP.2024.42.2.95](https://doi.org/10.1525/MP.2024.42.2.95).

## Phrase and style research

The user’s references—Mark Knight, Mau P, Chris Lake and HUGEL—guide a focus on short memorable club hooks, percussion space and recurring articulation. They are taste references, not artist presets. The [house/acid research brief](research/2026-11-11-house-acid.md) records direct artist interviews and original production lessons; the [Disco/Funk brief](research/2026-11-11-disco-funk.md) covers octave gestures, articulation and question/answer phrasing. Neither document claims a transcription of an unexamined recording.

| Evidence | Principle adopted | Limit |
| --- | --- | --- |
| Margulis and Simchy-Gross (2016), [institutional abstract](https://collaborate.princeton.edu/en/publications/repetition-enhances-the-musicality-of-randomly-generated-tone-seq/), DOI 10.1525/MP.2016.33.4.509 | Repetition deserves a deliberate role in composition. | Prior looping exposure produced a small rating effect; it does not prove that repeating any random bar makes a good bassline. |
| Bhandari and Colton (2024), [Motifs, Phrases, and Beyond](https://arxiv.org/html/2403.07995v1) | Plan phrase structure separately from individual events. | A review of symbolic generation, not a house-bass recipe or the source of our weights. |
| Hutchinson, [Non-Chord Tones](https://musictheory.pugetsound.edu/mt21c/NonChordTonesIntroduction.html), [Passing Tones](https://musictheory.pugetsound.edu/mt21c/PassingTones.html), [Neighbor Tones](https://musictheory.pugetsound.edu/mt21c/NeighborTones.html) | A connecting note has an actual destination; scale membership alone does not give it a harmonic role. | Our conservative tonic-triad framework is a product choice, not a prohibition on sustained sevenths or modal tones in real music. |
| Vitti/Stallworth, Berklee, [Bass Groove Basics](https://online.berklee.edu/takenote/bass-groove-basics-build-better-lines-with-qa-technique/) | Keep a recognizable statement and develop related answers. | One compositional teaching method, not a universal funk form. |
| TalkingBass, [Disco Bass Octaves](https://www.talkingbass.net/disco-bass-octaves/) | A recurring octave gesture can contribute disco identity. | Octaves alone do not define disco, and register changes need not occur on every note. |
| Roland, [TB-303 sequencing](https://articles.roland.com/mastering-the-tb-303-sequencer-in-roland-cloud/) | Repeat the pattern while selecting rests, contrasting gates and limited accents. | This MIDI-only device does not implement the 303 slide circuit or filter response. |

The [additional academic evidence](research/2026-11-11-structure-evidence.md) distinguishes each research finding from our implementation choices. The [old-engine audit](research/2026-11-11-engine-audit.md) found scale-correct notes alongside weak bar relationships, wide register jumps and density growth during Variation. That diagnosis motivated a composition change, rather than merely reducing the random range.

The later [sequence-model evidence](research/2026-11-11-sequence-model-evidence.md) and [style-refinement evidence](research/2026-11-11-style-refinement-evidence.md) examine the subsequent baseline at `595c60e`. Their code diagnoses and proposals are historical; the construction rules below describe the implemented refinement. The style note documents original lessons supporting related arrivals and anticipations, contrasting held and detached notes, independently chosen Acid accents, and shorter upper-octave Disco notes. These are teaching heuristics adapted into product rules, not measured genre frequencies or artist-specific models.

## Pitch vocabulary

The following are offsets in semitones from the selected tonic in twelve-tone equal temperament. “Minor” means **natural minor/Aeolian**, not harmonic or melodic minor. Scale membership constrains pitch classes; it does not guarantee compatibility with every chord in a song.

| Scale | Semitone offsets | Degrees relative to major |
| --- | --- | --- |
| Minor | 0, 2, 3, 5, 7, 8, 10 | 1, 2, ♭3, 4, 5, ♭6, ♭7 |
| Dorian | 0, 2, 3, 5, 7, 9, 10 | 1, 2, ♭3, 4, 5, 6, ♭7 |
| Phrygian | 0, 1, 3, 5, 7, 8, 10 | 1, ♭2, ♭3, 4, 5, ♭6, ♭7 |
| Major | 0, 2, 4, 5, 7, 9, 11 | 1, 2, 3, 4, 5, 6, 7 |
| Minor pentatonic | 0, 3, 5, 7, 10 | 1, ♭3, 4, 5, ♭7 |

These are standard scale definitions, checked against the academic open textbook *Open Music Theory*: [Diatonic Modes](https://viva.pressbooks.pub/openmusictheory/chapter/diatonic-modes/), [Introduction to Diatonic Modes](https://viva.pressbooks.pub/openmusictheory/chapter/intro-to-diatonic-modes-and-the-chromatic-scale/), and [Pentatonic Harmony](https://viva.pressbooks.pub/openmusictheory/chapter/pentatonic-harmony/). In Phrygian the lowered second is intentional; Dorian's natural sixth distinguishes it from natural minor.

## Controls and their musical meaning

Version 1.2 uses simpler labels in both interfaces: **Melody** is the existing Movement control, **Note length** is Gate, **Start on root** is Root Anchor, and **Root notes only** is Root Only. Max retains the previous internal parameter names for saved sets and automation. These label changes do not change the composition rules described above.

New instances start in C natural minor at C1 (MIDI note 36 in Live's octave convention) with a one-bar loop. Saved patterns and settings retain their original values. The historical 1.1 research audits keep their recorded settings, including their two-bar comparison condition; they are not reinterpreted using the new defaults.

- **Root notes only:** all newly generated notes have the selected tonic pitch class. C in several octaves is still root-only; C and G is not. With octave jumps disabled, root-only uses the selected base tonic pitch.
- **Start on root:** establish the tonic on the first generated onset. “First onset” need not be the bar's first grid position; an offbeat pattern must remain free to begin after the downbeat. This does not detect a chord's root from audio or infer a progression.
- **Octave jumps:** permit an upper-tonic response in the generated motif. Fresh notes stay within one octave of the selected base, rather than independently lifting every scale degree into a second octave. Preserved locks can remain outside that register.
- **Density:** controls how many onsets are likely. Density and syncopation are separate musical properties.
- **Melody:** raises the budget for third/fifth attacks within the jointly sampled core and for short, resolving scale-tone connectors. Repeated pitches, contour and the chosen destination influence each pitch decision. Connectors can also add an onset, so this control is not completely independent of Density.
- **Swing:** shifts alternating subdivisions as a timing rule. Arbitrary random jitter is not interchangeable with swing, and more timing deviation is not assumed to mean more groove.
- **Note length:** controls note duration relative to available space. Monophonic safety trims notes before the next onset and before the loop endpoint.
- **Locked notes:** preserve the user's existing decisions. A preserved note can be an exception to newly selected pitch controls; controls constrain regenerated notes. This precedence should be visible in help text.

Density and Melody are directly visible. More controls reveals Swing, Note length, Register and the pitch switches without altering their values. **Generate** applies pending settings. **Change a few notes**, the renamed Variation action, edits the current phrase and is unavailable while settings are pending. Its handler also guards against applying pending changes, so Generate is the explicit path to a newly configured phrase.

## Six style profiles: our compositional heuristics

These are practical directions for this instrument, not exhaustive definitions of genres or transcriptions of existing recordings. “Rolling” and “Offbeat” are rhythmic profiles rather than claims to independent historical genres.

| Style | Rhythm and articulation direction | Pitch direction |
| --- | --- | --- |
| Deep House | Choose an on-beat or delayed arrival and a related reply; an anticipated reply replaces the following beat attack in the core rhythm. Longer structural notes contrast with shorter answers. | Tonic-centered sequence with a chosen destination, contour bias and restrained third/fifth motion |
| Rolling | Construct a varied two-beat core, then repeat its rhythm, pitches, velocities and gates. Select one held-note role among detached attacks, constrained by available space. | Repeated tones and nearby triadic transitions within the sampled cell |
| Offbeat | At default Density, include all four eighth-note offbeats. Lower Density can thin that pulse; high Density adds short pickups attached to existing pulses. | Favor tonic repetition with a limited budget for third/fifth changes |
| Acid | Active sixteenths with accent placement selected separately from the held-note role; an accent need not be long | A repeated core with third/fifth and optional upper-tonic contrasts, conditioned on its endpoint |
| Disco | Favor eighth-note attacks and occasional pickups; upper-tonic notes use a shorter duration family than lower support notes | Tonic-triad support with an optional upper-tonic reply favored between beats |
| Funk | Relate answers to a selected arrival; allow a sixteenth anticipation to replace the later beat attack in the core. Emphasize structural attacks even when displaced. | Tonic-centered contour with repeated notes, third/fifth replies and optional upper tonic |

An Acid MIDI pattern alone does not reproduce a TB-303's timbre or slide circuit. MIDI velocity and note duration provide articulation that the chosen instrument may interpret differently. A monophonic nonoverlapping export does not promise legato glide.

## Construction rules

1. **Construct a fresh rhythm.** Style and Density determine an attack budget. Weighted sampling without replacement chooses optional subdivisions around structural support. Deep and Funk weight responses relative to their selected arrivals, including anticipations with an omitted following beat. Rolling constructs and repeats a two-beat cell; Offbeat preserves its between-beat pulse. These relationships assume a quarter-note beat reference; the engine does not read the user's kick track. No complete rhythm is retrieved from a bank.
2. **Sample a connected pitch sequence.** The core uses tonic, third, fifth and, when enabled, upper-tonic roles. Choose its first pitch, an endpoint weighted by the return to the first pitch, an arch-shaped contour bias and a Melody-dependent budget for third/fifth attacks. Compute the weight of feasible continuations backward, then sample forward using both the local transition and the remaining continuation weight. This jointly constructs the whole core rather than independently assigning one selected color by grid position. Repeats and nearby transitions receive explicit weight; the contour is a preference, not a required arch. The core is one bar, or the first two beats for Rolling before repetition. Root notes only removes third/fifth states; upper tonic does not consume the third/fifth budget.
3. **Articulate the idea.** Style-specific duration families distinguish sustained support, pulses and short ornaments. Deep/Funk structural arrivals receive emphasis even off the beat. Rolling repeats its held/detached contrast; Acid selects accent and held-note positions separately; Disco shortens the upper-tonic duration family. Note length scales these choices, and available room can shorten them further. Repeated statements retain their pitches, durations and velocities.
4. **Prepare connectors.** Melody allows a limited number of short, quieter nontriadic scale tones on weak sixteenth positions. Each must resolve by one or two semitones to the actual next triadic note within half a beat, including at the loop boundary. We call these connectors or approaches; only those satisfying the additional approach/departure conditions are passing or neighbor tones. A three-semitone pentatonic gap is not treated as a step.
5. **Develop a phrase.** Reuse the generated idea and derive limited ending responses. Longer phrases choose repeated-statement or alternating-answer forms. Changes concentrate in the latter half rather than replacing each bar independently.
6. **Apply user decisions and safety.** Exact locks take precedence. Resolve newly orphaned connectors, enforce monophony, register and loop limits, then save the actual result. Change a few notes edits a bounded subset of the current phrase without adding another layer of notes. When controls have changed, Generate rebuilds the phrase under those controls while retaining locks; Change a few notes stays unavailable until those settings are applied.

Numerical weights, budgets and contour preferences are authored design choices, not empirical measurements from the cited studies or a trained model. The constrained sampler covers the core pitch sequence; connectors and longer-form responses are subsequent rules, not a joint optimization of every final note. Structural checks can verify these rules, but do not establish listening superiority. Harmony assumes a tonic vamp because the device has no backing-chord input; agreement with an arbitrary song progression is not guaranteed. Modal notes remain available as resolving colors; this is not an exhaustive modal composition system.

A new generation is a new computation, not a promise that all possible outputs are unique. Very restrictive settings can produce the same short line by chance. Variation may intentionally retain information from the currently generated phrase, and locked notes are intentionally retained. Save the actual resulting notes so project recall and MIDI export reproduce what the user saw.

## Bibliography

1. Witek, Maria A. G., Eric F. Clarke, Mikkel Wallentin, Morten L. Kringelbach, and Peter Vuust. 2014. “Syncopation, Body-Movement and Pleasure in Groove Music.” *PLOS ONE* 9(4): e94446. DOI: 10.1371/journal.pone.0094446.
2. Butler, Mark J. 2001. “Turning the Beat Around: Reinterpretation, Metrical Dissonance, and Asymmetry in Electronic Dance Music.” *Music Theory Online* 7(6).
3. Witek, Maria A. G. 2017. “Filling In: Syncopation, Pleasure and Distributed Embodiment in Groove.” *Music Analysis* 36(1): 138–160. DOI: 10.1111/musa.12082.
4. Krumhansl, Carol L., and Edward J. Kessler. 1982. “Tracing the Dynamic Changes in Perceived Tonal Organization in a Spatial Representation of Musical Keys.” *Psychological Review* 89(4): 334–368. DOI: 10.1037/0033-295X.89.4.334.
5. Huron, David. 2001. “Tone and Voice: A Derivation of the Rules of Voice-Leading from Perceptual Principles.” *Music Perception* 19(1): 1–64. DOI: 10.1525/mp.2001.19.1.1.
6. *Open Music Theory*. “Diatonic Modes”; Chelsey Hamm, “Introduction to Diatonic Modes and the Chromatic Scale”; Mark Gotham and Bryn Hughes, “Pentatonic Harmony.” VIVA Open Publishing. Links above; consulted November 10, 2026.
7. Pachet, François, Pierre Roy, and Gabriele Barbieri. 2011. “Finite-Length Markov Processes with Constraints.” *Proceedings of IJCAI*, 635–642. DOI: 10.5591/978-1-57735-516-8/IJCAI11-113.
8. Sioros, George, Guy Madison, Diogo Cocharro, Anne Danielsen, and Fabien Gouyon. 2022. “Syncopation and Groove in Polyphonic Music: Patterns Matter.” *Music Perception* 39(5): 503–531. DOI: 10.1525/mp.2022.39.5.503.
9. Duncan, Sean-Lee, and Guido Orgs. 2024. “Move Your Body! Low-Frequency Amplitude and Syncopation Increase Groove Perception in House Music.” *Music Perception* 42(2): 95–108. DOI: 10.1525/MP.2024.42.2.95.
