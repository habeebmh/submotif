#!/usr/bin/env python3
"""Make an unapproved audition set from fresh calls to both shipping engines.

No corpus is read, and no pattern is chosen by a musical score. Every candidate
uses a predetermined seed. Audio is an original offline audition instrument,
not a synthesizer supplied by SUBMOTIF. Requires clang++, Node, NumPy, FFmpeg.
"""
import argparse
from concurrent.futures import ThreadPoolExecutor
import hashlib
import json
import math
from pathlib import Path
import struct
import subprocess
import tempfile
import wave

import numpy as np

ROOT = Path(__file__).resolve().parents[2]
STYLE_NAMES = [('deep', 'Deep House'), ('rolling', 'Rolling'), ('offbeat', 'Offbeat'),
               ('acid', 'Acid'), ('disco', 'Disco'), ('funk', 'Funk')]
SOURCE_PATHS = ['plugin/src/PatternEngine.cpp', 'plugin/src/PatternEngine.h', 'plugin/max/undertow-core.js',
                'web/scripts/render_showcase.py']
RATE, BPM, REPEATS = 44100, 126, 2
SEED_NAMESPACE = 'UNDERTOW by Habeeb / showcase review 1 / '

CPP = r'''
#include "PatternEngine.h"
#include <iomanip>
#include <iostream>
int main() {
    int id, style; std::uint32_t seed; std::cout << std::setprecision(17);
    while(std::cin >> id >> style >> seed) {
        undertow::Settings s; s.style=static_cast<undertow::Style>(style); s.bars=2;
        auto p=undertow::generate(s,seed);
        if(!undertow::isValid(p)) return 2;
        std::cout << "{\"id\":" << id << ",\"pattern\":{\"seed\":" << p.seed
                  << ",\"lengthBeats\":" << p.lengthBeats << ",\"notes\":[";
        for(int i=0;i<p.count;++i) {const auto& n=p.notes[i];if(i)std::cout<<',';
            std::cout << "{\"start\":" << n.start << ",\"duration\":" << n.duration
              << ",\"pitch\":" << n.pitch << ",\"velocity\":" << n.velocity
              << ",\"locked\":" << (n.locked?"true":"false") << '}';}
        std::cout << "]},\"midi\":["; auto data=undertow::midiFile(p);
        for(size_t i=0;i<data.size();++i){if(i)std::cout<<',';std::cout<<int(data[i]);}
        std::cout << "]}\n";
    }
    return std::cin.eof()?0:3;
}
'''
JS = r'''
const fs=require('node:fs'),core=require(process.argv[1]);
for(const row of JSON.parse(fs.readFileSync(0,'utf8'))){
  const settings=core.defaults();settings.style=row.style;settings.bars=2;
  console.log(JSON.stringify({id:row.id,settings,pattern:core.generate(settings,row.seed)}));
}
'''


def run(args, **kwargs):
    return subprocess.run(args, check=True, capture_output=True, **kwargs)


def source_hashes():
    return {name: hashlib.sha256((ROOT / name).read_bytes()).hexdigest() for name in SOURCE_PATHS}


def batch_fingerprint(metadata):
    """Identify the exact audition set, independently of approval or file paths.

    Sorted compact JSON is the version-one canonical representation. Source
    hashes belong to the original render, so a metadata-only migration keeps
    them intact and records its own script identity separately.
    """
    payload = dict(version=1, sources=metadata['sources'],
                   preview={key: metadata[key] for key in
                            ('seedNamespace', 'bpm', 'sampleRate', 'key', 'baseNote',
                             'baseMidi', 'bars', 'repetitions', 'sharedGain')},
                   candidates=[{key: row[key] for key in
                                ('candidateId', 'seed', 'style', 'settings', 'pattern')}
                               for row in metadata['candidates']])
    canonical = json.dumps(payload, sort_keys=True, separators=(',', ':'), ensure_ascii=True)
    return hashlib.sha256(canonical.encode('utf-8')).hexdigest()


def candidates():
    rows = []
    for style, (slug, label) in enumerate(STYLE_NAMES):
        for index in range(1, 11):
            candidate_id = f'{slug}-{index:02d}'
            seed = int.from_bytes(hashlib.sha256((SEED_NAMESPACE + candidate_id).encode()).digest()[:4], 'big') or 1
            rows.append(dict(id=len(rows), style=style, slug=slug, styleName=label,
                             number=index, candidateId=candidate_id, seed=seed))
    assert len({row['seed'] for row in rows}) == len(rows)
    return rows


def generate(rows):
    with tempfile.TemporaryDirectory(prefix='undertow-showcase-') as tmp:
        cpp, exe = Path(tmp) / 'adapter.cpp', Path(tmp) / 'adapter'
        cpp.write_text(CPP)
        run(['clang++', '-std=c++17', '-O2', '-I', str(ROOT / 'plugin/src'), str(cpp),
             str(ROOT / 'plugin/src/PatternEngine.cpp'), '-o', str(exe)])
        raw = run([str(exe)], input='\n'.join(f"{r['id']} {r['style']} {r['seed']}" for r in rows) + '\n', text=True).stdout
    native = [json.loads(line) for line in raw.splitlines()]
    max_rows = [json.loads(line) for line in run(['node', '-e', JS, str(ROOT / 'plugin/max/undertow-core.js')],
                input=json.dumps(rows), text=True).stdout.splitlines()]
    assert len(native) == len(max_rows) == 60
    for source, cpp_row, max_row in zip(rows, native, max_rows):
        assert source['id'] == cpp_row['id'] == max_row['id']
        left, right = cpp_row['pattern'], max_row['pattern']
        assert left['seed'] == right['seed'] == source['seed']
        assert left['lengthBeats'] == right['lengthBeats'] == 8
        assert len(left['notes']) == len(right['notes']) > 0
        for a, b in zip(left['notes'], right['notes']):
            assert all(a[k] == b[k] for k in ('pitch', 'velocity', 'locked'))
            assert all(abs(a[k] - b[k]) < 1e-12 for k in ('start', 'duration'))
        source.update(pattern=left, settings=max_row['settings'], midiBytes=cpp_row['midi'])
    return rows


def tick(beats):
    return math.floor(beats * 960 + .5)


def verify_midi(data, pattern):
    """Independently parse the actual native export, then compare every event."""
    assert data[:14] == b'MThd\0\0\0\6\0\0\0\1\3\xc0'
    assert data[14:18] == b'MTrk' and len(data) == int.from_bytes(data[18:22], 'big') + 22
    offset, time, notes, active, end = 22, 0, [], {}, None
    def vlq():
        nonlocal offset
        result = 0
        for _ in range(4):
            byte = data[offset]; offset += 1; result = (result << 7) | (byte & 127)
            if byte < 128: return result
        raise AssertionError('Bad VLQ')
    while offset < len(data):
        time += vlq(); status = data[offset]; offset += 1
        if status == 255:
            kind = data[offset]; offset += 1; count = vlq()
            if kind == 47: end = time
            offset += count
        else:
            assert status in (128, 144)
            pitch, velocity = data[offset:offset + 2]; offset += 2
            if status == 144 and velocity:
                assert pitch not in active
                active[pitch] = (time, velocity)
            else:
                start, v = active.pop(pitch)
                notes.append((start, time, pitch, v))
    expected = [(tick(n['start']), min(tick(pattern['lengthBeats']), max(tick(n['start']) + 1,
                 tick(n['start'] + n['duration']))), n['pitch'], n['velocity']) for n in pattern['notes']]
    assert not active and end == tick(pattern['lengthBeats']) and sorted(notes) == sorted(expected)


def add(buffer, signal, start):
    count = min(len(signal), len(buffer) - start)
    if count > 0: buffer[start:start + count] += signal[:count]


def render(pattern):
    """One original, harmonic pluck and drum bed shared by all 60 candidates."""
    spb = 60 / BPM
    length = pattern['lengthBeats'] * REPEATS * spb
    bass = np.zeros(round((length + .12) * RATE), dtype=np.float64)
    for repetition in range(REPEATS):
        for n in pattern['notes']:
            gate = n['duration'] * spb
            t = np.arange(round((gate + .025) * RATE)) / RATE
            phase = 2 * np.pi * (440 * 2 ** ((n['pitch'] - 69) / 12)) * t
            tone = (.65 * np.sin(phase) + .21 * np.sin(2 * phase) + .09 * np.sin(3 * phase)
                    + .035 * np.sin(4 * phase) + .015 * np.sin(5 * phase))
            envelope = np.minimum(t / .004, 1) * (.4 + .6 * np.exp(-t / .13))
            envelope *= np.where(t < gate, 1, np.maximum(0, 1 - (t - gate) / .025))
            add(bass, tone * envelope * (n['velocity'] / 127) * .46,
                round((n['start'] + repetition * pattern['lengthBeats']) * spb * RATE))
    drums = np.zeros_like(bass)
    rng = np.random.default_rng(2163)
    for beat in range(int(pattern['lengthBeats'] * REPEATS)):
        t = np.arange(round(.24 * RATE)) / RATE
        phase = 2 * np.pi * (49 * t + 75 * .018 * (1 - np.exp(-t / .018)))
        kick = np.sin(phase) * np.minimum(t / .001, 1) * np.exp(-t / .055) * .32
        add(drums, kick, round(beat * spb * RATE))
        t = np.arange(round(.06 * RATE)) / RATE
        noise = rng.uniform(-1, 1, len(t))
        hat = (noise - np.roll(noise, 1)) * np.exp(-t / .012) * .027
        add(drums, hat, round((beat + .5) * spb * RATE))
    tail = round(.025 * RATE)
    bass[-tail:] *= np.linspace(1, 0, tail)
    return bass, bass + drums


def write_wav(path, signal, gain):
    assert np.all(np.isfinite(signal)) and np.max(np.abs(signal * gain)) < .95
    pcm = np.round(signal * gain * 32767).astype('<i2')
    with wave.open(str(path), 'wb') as wav:
        wav.setnchannels(1); wav.setsampwidth(2); wav.setframerate(RATE)
        wav.writeframes(pcm.tobytes())


def encode(pair):
    source, dest = pair
    run(['ffmpeg', '-hide_banner', '-loglevel', 'error', '-y', '-i', str(source),
         '-codec:a', 'libmp3lame', '-b:a', '192k', '-map_metadata', '-1', str(dest)])
    decoded = run(['ffmpeg', '-hide_banner', '-loglevel', 'error', '-i', str(dest),
                   '-f', 'f32le', '-acodec', 'pcm_f32le', '-']).stdout
    samples = np.frombuffer(decoded, dtype='<f4')
    assert len(samples) > 0 and np.all(np.isfinite(samples)) and np.max(np.abs(samples)) < 1
    return float(np.max(np.abs(samples)))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', type=Path, default=ROOT / 'dist/showcase-candidates')
    parser.add_argument('--review', type=Path, default=ROOT / 'web/review')
    args = parser.parse_args()
    before = source_hashes()
    rows = generate(candidates())
    assert before == source_hashes(), 'Generator source changed during the run'
    out, review = args.output, args.review
    for path in (out / 'wav', out / 'midi', review / 'media', review / 'midi'):
        path.mkdir(parents=True, exist_ok=True)
    audio = {r['candidateId']: render(r['pattern']) for r in rows}
    peak = max(float(np.max(np.abs(stream))) for pair in audio.values() for stream in pair)
    gain = min(1, .82 / peak)
    jobs, compiled, compiled_solo, chapters = [], {}, {}, {}
    for slug, _ in STYLE_NAMES:
        compiled[slug], compiled_solo[slug], chapters[slug] = [], [], []
    for r in rows:
        name = r['candidateId']; raw_midi = bytes(r.pop('midiBytes'))
        verify_midi(raw_midi, r['pattern'])
        (out / 'midi' / f'{name}.mid').write_bytes(raw_midi)
        (review / 'midi' / f'{name}.mid').write_bytes(raw_midi)
        r['files'] = dict(midi=f'midi/{name}.mid', mix=f'media/{name}-mix.mp3', solo=f'media/{name}-solo.mp3')
        r['midiSha256'] = hashlib.sha256(raw_midi).hexdigest()
        r['approved'] = False
        for mode, stream in zip(('solo', 'mix'), audio[name]):
            wav = out / 'wav' / f'{name}-{mode}.wav'
            write_wav(wav, stream, gain)
            jobs.append((wav, review / 'media' / f'{name}-{mode}.mp3'))
        r['durationSeconds'] = len(audio[name][1]) / RATE
        r['noteCount'] = len(r['pattern']['notes'])
        pieces = compiled[r['slug']]
        cursor = sum(len(chunk) for chunk in pieces) / RATE
        chapters[r['slug']].append(dict(number=r['number'], candidateId=name, startSeconds=cursor,
                                         endSeconds=cursor + r['durationSeconds']))
        pieces.extend([audio[name][1], np.zeros(round(.65 * RATE))])
        compiled_solo[r['slug']].extend([audio[name][0], np.zeros(round(.65 * RATE))])
    style_metadata = []
    for slug, label in STYLE_NAMES:
        combined = np.concatenate(compiled[slug])
        wav = out / 'wav' / f'{slug}-all-ten.wav'
        write_wav(wav, combined, gain)
        mp3 = review / 'media' / f'{slug}-all-ten.mp3'
        jobs.append((wav, mp3))
        solo_wav = out / 'wav' / f'{slug}-all-ten-solo.wav'
        write_wav(solo_wav, np.concatenate(compiled_solo[slug]), gain)
        jobs.append((solo_wav, review / 'media' / f'{slug}-all-ten-solo.mp3'))
        matching = [r for r in rows if r['slug'] == slug]
        # Distinct exact patterns and coarse onsets/pitches are disclosed separately.
        exact = {json.dumps(r['pattern']['notes'], sort_keys=True) for r in matching}
        motifs = {tuple((round(n['start'] * 4), n['pitch']) for n in r['pattern']['notes']) for r in matching}
        assert len(exact) == 10, 'Duplicate complete candidates; inspect before presenting'
        style_metadata.append(dict(id=slug, name=label, compilation=f'media/{slug}-all-ten.mp3',
                                   compilationSolo=f'media/{slug}-all-ten-solo.mp3',
                                   chapters=chapters[slug], distinctPatterns=len(exact), distinctPitchRhythms=len(motifs)))
    with ThreadPoolExecutor(max_workers=4) as pool:
        decoded_peaks = list(pool.map(encode, jobs))
    metadata = dict(version=1, status='Awaiting artist selection; no candidates approved',
                    bpm=BPM, sampleRate=RATE, key='C natural minor', baseNote='C1', baseMidi=36,
                    bars=2, repetitions=REPEATS, sharedGain=gain, pcmPeak=peak * gain,
                    decodedMp3Peak=max(decoded_peaks), seedNamespace=SEED_NAMESPACE,
                    sources=before, styles=style_metadata, candidates=rows,
                    previewNote='Original offline bass and drum synthesis for listening only. UNDERTOW generates MIDI and uses your own instrument.',
                    validation=dict(freshCppGenerations=60, maxParityPatterns=60, exactNativeMidiExports=60,
                                    validatedMp3Files=len(jobs), clippingDetected=False, seedsReplaced=0))
    metadata['batchFingerprintAlgorithm'] = 'sha256-canonical-json-v1'
    metadata['batchFingerprint'] = batch_fingerprint(metadata)
    text = json.dumps(metadata, indent=2) + '\n'
    (out / 'manifest.json').write_text(text)
    (review / 'manifest.json').write_text(text)
    # A local script also makes the reviewer usable when opened directly from disk.
    (review / 'candidates.js').write_text('window.UNDERTOW_AUDITIONS = ' + json.dumps(metadata) + ';\n')
    guide = ['# UNDERTOW: 60 candidates for approval', '',
             'Choose one candidate per style in `web/review/index.html`. Nothing is selected or approved by default.', '',
             'Each candidate is a fresh call to the current C++ weighted generator, checked against the Max generator.',
             'All examples: C natural minor, C1 base, two bars, 126 BPM, played twice. Plugin defaults remain one bar.',
             'Identical original offline bass/drum patch and shared gain keep the comparison consistent. No artist audio or third-party samples.',
             'The plug-in contains no synthesizer. Use the exact MIDI exports with your own instrument to judge tone in context.', '',
             '| Style | Compilation | Distinct complete phrases | Distinct pitch/rhythm motifs |', '|---|---|---:|---:|']
    for style in style_metadata:
        guide.append(f"| {style['name']} | `{style['id']}-all-ten.wav` | {style['distinctPatterns']} | {style['distinctPitchRhythms']} |")
    guide += ['', 'The manifest contains every seed, setting, note, source-file hash, and compilation timestamp.',
              'Candidate numbers are labels, not quality rankings. No seed was replaced or manually edited.',
              f'Validation: 60 C++/Max pattern matches; 60 parsed native MIDI exports; {len(jobs)} MP3s decoded without clipping.']
    (out / 'README.md').write_text('\n'.join(guide) + '\n')
    print(json.dumps({'candidates': 60,
                      'styles': [{k: s[k] for k in ('id', 'distinctPatterns', 'distinctPitchRhythms')} for s in style_metadata], 'sharedGain': gain,
                      'pcmPeak': peak * gain, 'decodedMp3Peak': max(decoded_peaks),
                      'review': str(review / 'index.html'), 'validation': metadata['validation']}, indent=2))


if __name__ == '__main__':
    main()
