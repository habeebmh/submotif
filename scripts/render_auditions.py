#!/usr/bin/env python3
"""Render fixed-seed old/new composition comparisons, not a plug-in synthesizer.

Requires NumPy only for this offline review tool. Uses original mathematical
oscillators/noise, no samples, and one shared gain across every comparison.
"""
import argparse
import gzip
import json
import math
from pathlib import Path
import wave

import numpy as np

STYLES = ['deep', 'rolling', 'offbeat', 'acid', 'disco', 'funk']


def read_corpus(folder):
    with gzip.open(folder / 'corpus.jsonl.gz', 'rt') as source:
        return {(r['style'], r['seedIndex']): r for r in map(json.loads, source)
                if r['engine'] == 'cpp' and r['profile'] == 'default'}


def add_signal(buffer, signal, offset):
    count = min(len(signal), len(buffer) - offset)
    if count > 0:
        buffer[offset:offset + count] += signal[:count]


def render(pattern, sample_rate, bpm):
    seconds_per_beat = 60 / bpm
    # Two repetitions let the listener hear both internal structure and loop seam.
    length = pattern['lengthBeats'] * 2 * seconds_per_beat
    audio = np.zeros(round((length + .15) * sample_rate), dtype=np.float64)
    for repetition in range(2):
        for note in pattern['notes']:
            onset = (note['start'] + repetition * pattern['lengthBeats']) * seconds_per_beat
            gate = note['duration'] * seconds_per_beat
            t = np.arange(round((gate + .025) * sample_rate)) / sample_rate
            frequency = 440 * 2 ** ((note['pitch'] - 69) / 12)
            phase = 2 * np.pi * frequency * t
            # Same clean, slightly rounded pluck for ALL styles and both engines.
            tone = (.72 * np.sin(phase) + .19 * np.sin(2 * phase)
                    + .075 * np.sin(3 * phase) + .015 * np.sin(4 * phase))
            envelope = np.minimum(t / .004, 1) * (.40 + .60 * np.exp(-t / .13))
            envelope *= np.where(t < gate, 1, np.maximum(0, 1 - (t - gate) / .025))
            signal = tone * envelope * (note['velocity'] / 127) * .42
            add_signal(audio, signal, round(onset * sample_rate))
    # Identical minimal four-on-the-floor accompaniment: original synthetic sounds.
    random = np.random.default_rng(2163)
    for beat in range(round(pattern['lengthBeats'] * 2)):
        t = np.arange(round(.24 * sample_rate)) / sample_rate
        phase = 2 * np.pi * (49 * t + 75 * .018 * (1 - np.exp(-t / .018)))
        kick = np.sin(phase) * np.minimum(t / .001, 1) * np.exp(-t / .055) * .32
        add_signal(audio, kick, round(beat * seconds_per_beat * sample_rate))
        t = np.arange(round(.06 * sample_rate)) / sample_rate
        noise = random.uniform(-1, 1, len(t))
        hat = (noise - np.roll(noise, 1)) * np.exp(-t / .012) * .027
        add_signal(audio, hat, round((beat + .5) * seconds_per_beat * sample_rate))
    # Fade only the very end, preserving the internal loop boundary for evaluation.
    tail = round(.025 * sample_rate)
    audio[-tail:] *= np.linspace(1, 0, tail)
    return audio


def midi_bytes(pattern):
    def vlq(value):
        if value < 0:
            raise ValueError('MIDI event delta cannot be negative')
        output = [value & 127]
        while (value := value >> 7):
            output.insert(0, (value & 127) | 128)
        return bytes(output)
    end_tick = math.floor(pattern['lengthBeats'] * 960 + .5)
    events = []
    for n in pattern['notes']:
        start = math.floor(n['start'] * 960 + .5)
        end = min(end_tick, max(start + 1, math.floor((n['start'] + n['duration']) * 960 + .5)))
        if not 0 <= start < end <= end_tick:
            raise ValueError('MIDI note is outside the loop')
        events.append((start, 1, bytes([144, n['pitch'], n['velocity']])))
        events.append((end, 0, bytes([128, n['pitch'], 0])))
    track = bytearray([0, 255, 88, 4, 4, 2, 24, 8])
    previous = 0
    for tick, _, data in sorted(events):
        track += vlq(tick - previous) + data
        previous = tick
    track += vlq(end_tick - previous) + bytes([255, 47, 0])
    return b'MThd\x00\x00\x00\x06\x00\x00\x00\x01\x03\xc0MTrk' + len(track).to_bytes(4, 'big') + track


def write_wav(path, samples, sample_rate, gain):
    pcm = np.round(samples * gain * 32767).astype('<i2')
    with wave.open(str(path), 'wb') as destination:
        destination.setnchannels(1)
        destination.setsampwidth(2)
        destination.setframerate(sample_rate)
        destination.writeframes(pcm.tobytes())


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--before', type=Path, required=True)
    parser.add_argument('--after', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--bpm', type=float, default=126)
    args = parser.parse_args()
    if not 60 <= args.bpm <= 180:
        parser.error('bpm must be between 60 and 180')
    old, new = read_corpus(args.before), read_corpus(args.after)
    args.output.mkdir(parents=True, exist_ok=True)
    (args.output / 'midi').mkdir(exist_ok=True)
    rate = 44100
    pause = np.zeros(round(.65 * rate))
    comparisons, newer, chapters = {}, {}, {}
    for style in STYLES:
        chunks, after_chunks, labels, cursor = [], [], [], 0
        # Always use first three successive corpus seeds; never pick attractive cases.
        for index in range(3):
            for edition, corpus in [('before', old), ('after', new)]:
                pattern = corpus[(style, index)]['pattern']
                signal = render(pattern, rate, args.bpm)
                labels.append({'seconds': round(cursor / rate, 3), 'edition': edition,
                               'seed': pattern['seed'], 'notes': len(pattern['notes'])})
                chunks.extend([signal, pause])
                cursor += len(signal) + len(pause)
                if edition == 'after':
                    after_chunks.extend([signal, pause])
                name = f'{style}-{edition}-{index + 1}-seed{pattern["seed"]}.mid'
                (args.output / 'midi' / name).write_bytes(midi_bytes(pattern))
        comparisons[style] = np.concatenate(chunks)
        newer[style] = np.concatenate(after_chunks)
        chapters[style] = labels
    peak = max(float(np.max(np.abs(a))) for a in comparisons.values())
    gain = min(1, .85 / peak)
    for style in STYLES:
        write_wav(args.output / f'{style}-before-after.wav', comparisons[style], rate, gain)
        write_wav(args.output / f'{style}-revised.wav', newer[style], rate, gain)
    metadata = {'bpm': args.bpm, 'sample_rate': rate, 'shared_gain': gain,
                'peak_after_gain': peak * gain, 'chapters': chapters,
                'scope': 'Offline composition audition. Same original synthetic bass/drums/gain for every example. No artist audio; not a sound included in the MIDI plug-in.'}
    (args.output / 'audition-manifest.json').write_text(json.dumps(metadata, indent=2) + '\n')
    guide = ['# Bassline comparisons', '', f'All examples: C natural minor, {args.bpm:g} BPM, identical bass patch and drums.',
             'Each comparison plays **old, revised**, then repeats that order for the next two seeds. Each phrase loops twice.',
             'The revised-only files contain the same three new phrases. Seeds are the first three in the audit corpus, not hand-picked.',
             'These are original offline demo sounds; the plug-in continues to use your own instrument.', '', '| Style | Before / after | Revised only |', '|---|---|---|']
    for style in STYLES:
        guide.append(f'| {style.title()} | [{style}-before-after.wav]({style}-before-after.wav) | [{style}-revised.wav]({style}-revised.wav) |')
    guide += ['', 'The `midi/` folder contains each exact two-bar phrase. `audition-manifest.json` lists timestamps and seeds.',
              'Structural measurements are not listening scores. Judge these in your track as well as against this neutral accompaniment.']
    (args.output / 'README.md').write_text('\n'.join(guide) + '\n')
    print(json.dumps({'output': str(args.output), 'comparisons': len(comparisons),
                      'midi_files': len(list((args.output / 'midi').glob('*.mid'))),
                      'peak_after_gain': peak * gain, 'sample_rate': rate}, indent=2))


if __name__ == '__main__':
    main()
