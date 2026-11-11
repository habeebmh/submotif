# Sequence generation evidence and a practical next experiment

Research date: 2026-11-11. Historical baseline: the procedural C++/ES5 composition engines at repository HEAD `595c60e`, before the subsequent refinement. Code diagnoses, source-line references, hashes and probe results below describe that baseline, not the current source. Proposals remain a research record; see [MUSIC_THEORY.md](../MUSIC_THEORY.md) for implemented behavior. This note changed no product code, tests, audio, website, packaging, or installation.

## Main recommendation

Keep runtime construction and explicit motif development, but prototype **sampling a complete, feasible sequence of musical roles and transitions**, rather than choosing one colour pitch and assigning it by fixed grid tests. Condition the sequence on a deliberately chosen destination and loop connection. Use authored style rules initially; describe them as authored rules. A small constrained sampler is feasible without a neural network or a stored bassline bank, but its musical benefit remains a listening hypothesis.

Do not repeat the old diagnosis that the current engine has no motif memory. [MUSIC_THEORY.md](../MUSIC_THEORY.md), the [structure note](2026-11-11-structure-evidence.md), and the [revision results](2026-11-11-revision-results.md) already document generated motifs, repetitions, restricted connectors, shared engine rules, and bounded Variation. Those are real improvements in structure. The remaining question is whether the initial idea, its development, and its articulation are worth repeating. Better structural audit scores do not answer that question.

## What the research establishes

### Constrained sequence sampling is different from selecting a locally legal note

Pachet and Roy, **“Markov constraints: steerable generation of Markov sequences,”** *Constraints* 16, 148–172 (2011; online 2010), DOI [10.1007/s10601-010-9101-4](https://doi.org/10.1007/s10601-010-9101-4), formulate sequence construction as constraint satisfaction with branch-and-bound optimization. The method can combine fixed/variable-order Markov information with additional requirements such as anchors and contour. Its validation addresses expressiveness and scalability; it does not validate house-bass taste. Arbitrary constraints are more costly than greedy generation. [Author-hosted full paper](https://www.francoispachet.fr/wp-content/uploads/2021/01/pachet-09c.pdf), especially §§3–4.

The distinct follow-up by Pachet, Roy, and Barbieri, **“Finite-Length Markov Processes with Constraints,”** IJCAI 2011, DOI [10.5591/978-1-57735-516-8/IJCAI11-113](https://doi.org/10.5591/978-1-57735-516-8/IJCAI11-113), is more directly useful here. For unary and adjacent-pair constraints within the model's scope, it filters infeasible continuations and propagates normalization backward. Sampling then preserves the relative probabilities of feasible sequences. Simply deleting illegal transitions and normalizing each row independently does not preserve that distribution. Compilation has an O(L × K²) normalization step; subsequent sampling is O(L × K). Its broader-constraint extension does not retain the same probability guarantee. [Full paper, §§4.1–4.7](https://www.francoispachet.fr/wp-content/uploads/2021/01/pachet-11b.pdf).

**Transfer:** destination-aware sampling is technically supported. The papers do not supply Submotif's transition weights. A hand-authored transition matrix is a procedural stochastic model, not a trained imitation of the six styles. The general CSP optimizer and the narrower probability-preserving sampler must not be presented as interchangeable algorithms.

### Expectation concerns context, not just scale membership

Pearce and Wiggins, **“Auditory Expectation: The Information Dynamics of Music Perception and Cognition,”** *Topics in Cognitive Science* 4, 625–652 (2012), DOI [10.1111/j.1756-8765.2012.01214.x](https://doi.org/10.1111/j.1756-8765.2012.01214.x), describe IDyOM's variable-order sequence prediction, smoothing, and multiple viewpoints, including interval, timing, and scale degree. Long-term musical exposure and short-term learning within a piece contribute predictions. The paper distinguishes expectation/segmentation results from more speculative aesthetics and creativity applications. [Author-hosted full paper](https://www.marcus-pearce.com/assets/papers/PearceWiggins2012.pdf), §§3–6.

The original experiment by Pearce et al., **“Unsupervised statistical learning underpins computational, behavioural, and neural manifestations of musical expectation,”** *NeuroImage* 50, 302–313 (2010), DOI [10.1016/j.neuroimage.2009.12.019](https://doi.org/10.1016/j.neuroimage.2009.12.019), found correspondence between model surprise and expectedness ratings/neural responses. Its 28 hymn melodies had rhythm regularized to focus on pitch. The authors discuss interval size as a possible acoustic confound. This is expectation evidence, not a test of enjoyable basslines. [Full paper: methods, results, and discussion](https://www.marcus-pearce.com/assets/papers/PearceEtAlNeuroImage2010.pdf).

**Transfer:** represent recent interval/role context and phrase position explicitly. Neither paper justifies a universal “optimal surprise” target, maximizing entropy, or treating probability under our own hand-set weights as a listener model. Full IDyOM needs appropriate exposure data and model construction; adding a two-note memory does not implement IDyOM.

### Contour is more than taking small steps

Schellenberg, Adachi, Purdy, and McKinnon, **“Expectancy in Melody: Tests of Children and Adults,”** *Journal of Experimental Psychology: General* 131, 511–537 (2002), DOI [10.1037/0096-3445.131.4.511](https://doi.org/10.1037/0096-3445.131.4.511), compared continuation judgments and sung continuations. Proximity mattered across ages; older listeners also showed expectations involving directional reversal and symmetric returns. Their two-factor account matched or exceeded the larger implication-realization model in the reported comparisons. Participants had Western musical exposure; the tests were not bass grooves. [Author-hosted full paper](https://sites.utm.utoronto.ca/sites/sites.utm.utoronto.ca.glenn_website/files/download/JEPGeneral.pdf), experiments 1–2 and general discussion.

**Transfer:** remembering the last two sounding pitches permits repeat/step/leap-and-return distinctions. A modest preference for a return after a leap is worth testing; a mandatory reversal after every leap is not established. Disco octave alternation and intentional Acid register contrasts need their own role context. The paper does not say “ban sevenths,” “keep every leap below a fifth,” or “every melody must be arch shaped.”

### Groove findings do not justify a scalar quality function

Witek et al., **“Syncopation, Body-Movement and Pleasure in Groove Music,”** *PLOS ONE* 9, e94446 (2014), studied 66 participants and 50 synthesized two-bar drum breaks at 120 BPM with a steady eighth-note hi-hat; no microtiming was included. Intermediate syncopation attracted higher ratings in these stimuli; audio entropy was a poor predictor. This is already cited in the project, but its stimulus context matters. [Full methods and results](https://journals.plos.org/plosone/article?id=10.1371/journal.pone.0094446).

The **2015 correction**, DOI [10.1371/journal.pone.0139409](https://doi.org/10.1371/journal.pone.0139409), repairs stimulus indexing/notational transcripts and supplies corrected supplements, including the syncopation-index text. The authors state that the findings and interpretation are unaffected. Any reuse of its notation or metric should consult the corrected files. [Full correction](https://journals.plos.org/plosone/article?id=10.1371/journal.pone.0139409).

Sioros, Madison, Cocharro, Danielsen, and Gouyon, **“Syncopation and Groove in Polyphonic Music: Patterns Matter,”** *Music Perception* 39, 503–531 (2022), DOI [10.1525/mp.2022.39.5.503](https://doi.org/10.1525/mp.2022.39.5.503), provides a useful counterweight. Thirty-five listeners judged versions of ten funk/rock excerpts containing bass, drums, and guitar or keyboards. Original syncopation improved groove relative to its removal; pseudorandom syncopation did not significantly improve groove over the de-syncopated versions. The second study analyzed instrument roles, metrical placement, counter-rhythms, and pickups. Its proposed causal explanations are explicitly hypotheses for further tests, not experimentally established construction rules. [Full methods, discussion, and conclusion](https://online.ucpress.edu/mp/article/39/5/503/182325/Syncopation-and-Groove-in-Polyphonic-MusicPatterns).

**Transfer:** compare structured relationships among attacks and rests in a drum context. Neither independent offbeat weights nor a middle-valued syncopation sum guarantee groove. These results also do not license a universal rule that bass must avoid kick attacks.

### BassNet sets a contextual limit, not a small-runtime recipe

Grachten, Lattner, and Deruty, **“BassNet: A Variational Gated Autoencoder for Conditional Generation of Bass Guitar Tracks with Learned Interactive Control,”** *Applied Sciences* 10, 6627 (2020), DOI [10.3390/app10186627](https://doi.org/10.3390/app10186627), models relationships between existing audio and generated bass, with a learned two-dimensional control space. It uses spectro-temporal audio representations and a trained neural architecture. Its validation/case studies are not a controlled test establishing superiority for the six Submotif styles. [Original article](https://www.mdpi.com/2076-3417/10/18/6627); [authors' project and methods overview](https://sonycslparis.github.io/bassnet/).

Access qualification: the paper's cached body exposed its introduction, model description, and conclusions; live article/PDF requests returned 429/403 or were inaccessible. The authors' full project page was accessible. This note therefore uses BassNet only for its plainly documented conditioning architecture and workflow, not numerical performance claims or a claimed full replication review. Other core sources above were available as full article/PDF text and their methods and limitations were consulted.

**Transfer:** bass coherence is partly a relationship with other parts. Submotif currently has no chord or drum input, so no sequence-model upgrade can infer the user's unprovided harmony. An optional future beat mask/chord plan would add information; replacing the current engine with “BassNet-inspired” hand rules would not implement BassNet.

## Current-engine limits confirmed in source

Inspected files and SHA-256 hashes:

- `plugin/src/PatternEngine.cpp`: `1a9da4d0b35231749f316e75ea7a1f95fe5e9cfcc15c4162613e335fe9db6f41`
- `plugin/max/undertow-core.js`: `646111bc916df4e1ffb6a0976ed999a891294bbb36a19dcc11e8d25edfe00807`

1. **One chosen colour governs the whole initial motif.** C++ `makeMotif` at line 224 chooses either the fifth or third at line 228, then distributes it using fixed step comparisons. ES5 does the same at line 206. Connectors explicitly exclude triadic notes. Thus an unlocked newly generated one-bar motif cannot contain both third and fifth, even with Melody at maximum. This is a reachability limit, not evidence that every good bassline needs both.
2. **Deep's upper-tonic role is computed but not realized.** `upperRole` exists, but the shared Deep/Acid branch applies it only for Acid above Melody 0.6. Deep's initial motif cannot use the upper tonic. Multi-bar responses or Variation can introduce it through `changedTriadPitch`. Enabling Octave jumps can still change other seeded output because it changes random-number consumption; do not report that the toggle has literally no effect.
3. **Five profiles force a downbeat onset regardless of Start on root.** `makeRhythm` line 73 fixes step 0 for Deep, Rolling, Acid, Disco, and Funk. Turning Start on root off changes pitch eligibility, not opening-rest eligibility. This limits delayed entrances and anticipation-based identities; it is not proof that a downbeat is undesirable.
4. **Longer-form responses are local substitutions.** `response` line 274 edits one or two late notes; `changedTriadPitch` samples equally among the permitted alternatives without considering arrival direction or the next structural target. Connector repair keeps local approaches valid, but triadic replacements have no contour objective. The one-bar default never plays `answerB` or `answerC` at all (`generate`, lines 409–420).
5. **The “zero unresolved nontriads” result is partly guaranteed by a narrow vocabulary policy.** All nontriadic generated notes must be short weak-sixteenth approaches. A sustained modal sixth/seventh or a two-note scalar connection is structurally unavailable. That is an intentional documented policy; its audit success should not be treated as proof that these excluded gestures would be musically worse.

A read-only probe of the current Max core used 256 dispersed seeds per style: `(0xa341316c + i × 2654435761) mod 2^32`, `i=0…255`; one bar, C minor, MIDI36 base, Density .55, Melody 1, Swing .12, Gate .65, Start on root false, Root notes only false, Octave jumps true. Directly calling `core.generate` yielded:

| Profile | Phrases with upper tonic | Phrases with both third and fifth | Phrases beginning at beat 0 |
| --- | ---: | ---: | ---: |
| Deep | 0/256 | 0/256 | 256/256 |
| Rolling | 114/256 | 0/256 | 256/256 |
| Offbeat | 67/256 | 0/256 | 0/256 |
| Acid | 118/256 | 0/256 | 256/256 |
| Disco | 256/256 | 0/256 | 256/256 |
| Funk | 72/256 | 0/256 | 256/256 |

This confirms the described implementation limits in that sample. It is not a quality ranking, not a fresh C++ parity test, and not a new stored pattern collection. No generated MIDI or audition audio was written.

## Proposed implementation experiments, not paper-validated recipes

### 1. A bounded role-and-contour sampler

Retain procedurally chosen onsets for the first experiment so pitch logic can be assessed independently. Choose a few event roles—support, repeated pulse, answer, approach—and a final destination/loop relation. Give each event an allowed pitch domain, then sample transitions considering prior pitch and, where useful, the preceding interval direction. Permit both third and fifth when the chosen contour calls for them. Keep Root notes only and locks as hard constraints. Add modal tones only through explicit role policies that can be auditioned independently.

For a fixed first note and fixed role plan, an implementation can store a nonnegative local transition weight `w_i(a,b)` that includes allowed-domain/transition masks. Set terminal continuation mass `B_last(a)` to the weight/validity of returning from `a` to the first note. Compute backward:

```text
B_i(a) = sum over b of w_i(a,b) * B_(i+1)(b)
P(next=b | current=a, feasible ending) ∝ w_i(a,b) * B_(i+1)(b)
```

This weights a choice by the feasible future it leaves. With arbitrary nonnegative authored weights it samples the distribution defined by their product; with suitable Markov probabilities it yields the corresponding conditioned model. Neither interpretation turns those weights into perceptual truth. If the first note is free, compute completion masses for possible first notes and include those masses when sampling the start; uniform start selection followed by conditioning would define a different distribution.

For `N` events and `K` states, the simple fixed-start chain requires O(NK²) work and O(NK) backward storage; computing all `K` possible starts adds up to another factor of `K`. At N≤16 and K≈8–32 this is a plausible small-runtime prototype, not a measured timing claim. Packing recent direction or counters into state increases K. Normalize backward layers to avoid numerical underflow; handle zero completion mass explicitly. Keep deterministic loop order and identical arithmetic/seed conventions across C++ and ES5. Construct on Generate, outside time-critical MIDI emission, using bounded storage.

Locks with unusual pitches/timing may make a proposed shape impossible. Preserve them and relax only authored soft contour preferences; never silently move a lock. Report an internal fallback reason for auditing. Generate around actual lock targets rather than repairing unrelated ornamentation afterward.

### 2. Construct relationships in rhythm, then articulate their roles

In a separate experiment, replace some independent optional-step selection with a tiny finite-state rhythm grammar: pulse continuation, gap, pickup into an already planned target, short run, and release. These are construction operators, not complete stored bars. A counter can keep an onset budget while a state remembers the previous gap/run. The 16-step search is small, but the combined state size must be measured rather than assumed.

Allow more than one onset-phase policy for Deep/Funk/Acid when appropriate; preserve the deliberately narrower Offbeat identity. For Rolling, vary a generated cell through controlled displacement or a terminal rest as alternatives to mandatory exact two-beat mirroring. Keep Disco octave gestures and Acid accents as explicit relationships. Pair duration/velocity choices with each gesture so a pickup sounds like one and a support note has intentional weight. These are authored hypotheses; Sioros et al. does not validate this grammar.

### 3. Defer learned models until their data and purpose are defined

An offline trained, smoothed order-1/2 transition model could eventually export compact numeric tables to both runtimes. Relative pitch, interval, metrical phase, and role are possible features. A full joint table grows quickly and sparse counts become unreliable; factoring features loses interactions. Training requires a suitable, legally usable corpus with clear style/context labels and held-out songs or artists. Do not train on the engine's own outputs and claim external musical evidence. Do not optimize likelihood alone or claim that a trained model cannot reproduce corpus fragments. No training data or model is added here.

## Practical evaluation that can reject the hypothesis

Keep separate **mechanical validation** and **human judgment**. Mechanical checks can establish scale/lock/monophony safety, determinism/parity, runtime bounds, feasible endings, control reachability, and fallback rates. They cannot establish musical quality.

Use a preregistered seed list and keep every eligible result for the initial comparison, including failures. Compare the current engine against (a) destination/contour changes only, (b) rhythm relationships only, and then (c) the combination. Match settings, tempo, sound, drum/chord context, and playback level within each pair; balance A/B order and hide the version. Do not regenerate selectively until the candidate wins. Freeze tuning before a second held-out seed set.

Test one-bar defaults first because that is the new-instance experience, then two-/four-bar phrases and restrictive control cases. Cover all six profiles, at least restrained and high Melody, octave on/off, and representative locks. Include bass alone, a fixed club drum backing, and explicitly specified harmonic backing. The last condition tests fit to a known context; it cannot establish compatibility with arbitrary songs. Repeat promising comparisons with a second bass sound to expose dependence on envelopes and velocity response.

Ask distinct questions: which line would the listener keep; whether the loop ending feels intentional; whether its rhythm supports the backing; whether the answer feels related; and where a note or articulation feels wrong. Permit “neither” and ties. A usable-line rate under a fixed audition budget and edits required before use can be practical outcomes. They remain listener/task reports, not universal quality scores. Record taste/style familiarity and explain disagreements instead of averaging them away.

Do not force more pitch classes, exact repetition, minimum surprise, maximum syncopation, or zero nontriads. A repeated tonic can be excellent; a more elaborate line can fail. If the new sampler passes constraints but loses blind preference or takes more editing, reject or revise it. The current user's dissatisfaction is already a reason to reopen listening, not a reason to relabel old structural metrics as proof of success.
