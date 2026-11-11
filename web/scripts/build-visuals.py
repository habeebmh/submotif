#!/usr/bin/env python3
"""Render site graphics from the approved MIDI and audio. Never change recordings."""
import array
import hashlib
import html
import json
from pathlib import Path
import subprocess

WEB = Path(__file__).resolve().parents[1]
OUT = WEB / 'assets/graphics'
OUT.mkdir(exist_ok=True)
approval = json.loads((WEB / 'assets/audio/approved-manifest.json').read_text())
manifest = json.loads((WEB / 'review/manifest.json').read_text())
assert approval['sourceBatchFingerprint'] == manifest['batchFingerprint']


def svg(body, width, height, title):
    return (f'<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 {width} {height}" '
            f'role="img" preserveAspectRatio="none"><title>{html.escape(title)}</title>{body}</svg>\n')


def notes(pattern, width=640, height=180, color='#b9dd65', grid=True):
    low = min(n['pitch'] for n in pattern['notes']) - 1
    high = max(n['pitch'] for n in pattern['notes']) + 1
    rows = high - low + 1
    body = ''
    if grid:
        for beat in range(33):
            x = beat * width / 32
            body += f'<path d="M{x:g} 0V{height}" stroke="#344039" stroke-opacity="{.7 if beat%4==0 else .25}"/>'
        for row in range(rows + 1):
            y = row * height / rows
            body += f'<path d="M0 {y:g}H{width}" stroke="#344039" stroke-opacity=".3"/>'
    for n in pattern['notes']:
        x = n['start'] / pattern['lengthBeats'] * width
        y = (high - n['pitch']) / rows * height
        w = n['duration'] / pattern['lengthBeats'] * width
        h = max(3, height / rows - 3)
        body += f'<rect x="{x:.2f}" y="{y+1.5:.2f}" width="{w:.2f}" height="{h:.2f}" rx="2" fill="{color}" fill-opacity="{.5+n["velocity"]/254:.3f}"/>'
    return body


records = []
for selection in approval['selections']:
    assert selection['audioApproved'] and selection['midiApproved']
    candidate = next(c for c in manifest['candidates'] if c['candidateId'] == selection['id'])
    pattern = json.loads(json.dumps(candidate['pattern']))
    if selection.get('transformation'):
        for note in pattern['notes']:
            note['pitch'] += selection['transformation']['semitones']
    key = selection['id'].split('-')[0]
    (OUT / (key + '-notes.svg')).write_text(svg(notes(pattern), 640, 180, selection['style'] + ' approved MIDI phrase'))
    audio = selection['audio']['mix']
    path = WEB / 'assets/audio' / audio['path']
    assert hashlib.sha256(path.read_bytes()).hexdigest() == audio['sha256']
    pcm = array.array('f', subprocess.check_output([
        'ffmpeg', '-v', 'error', '-i', str(path), '-ac', '1', '-ar', '8000', '-f', 'f32le', '-']))
    peaks = [max(abs(x) for x in pcm[i*len(pcm)//144:(i+1)*len(pcm)//144]) for i in range(144)]
    maximum = max(peaks)
    peaks = [round(p/maximum, 4) for p in peaks]
    body = ''.join(f'<rect x="{i*4.4:.1f}" y="{32-p*29:.2f}" width="2.4" height="{max(1,p*58):.2f}" rx="1.2" fill="#a9b2a7"/>' for i,p in enumerate(peaks))
    (OUT / (key + '-wave.svg')).write_text(svg(body, 640, 64, selection['style'] + ' recording waveform'))
    records.append({'id':selection['id'], 'audioSha256':audio['sha256'], 'peaks':peaks})

# A wide, quiet visual signature made from the selected Rolling phrase.
rolling = next(c for c in manifest['candidates'] if c['candidateId'] == 'rolling-06')['pattern']
(OUT / 'rhythm-strip.svg').write_text(svg(notes(rolling, 1200, 160, grid=False), 1200, 160, 'Rolling bassline MIDI'))
(OUT / 'provenance.json').write_text(json.dumps({'batchFingerprint':manifest['batchFingerprint'], 'waveforms':records}, indent=2)+'\n')
print('Rendered six exact MIDI graphics and six audio-derived waveforms; approved media unchanged.')
