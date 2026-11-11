#!/usr/bin/env python3
"""Offline descriptive audit of the real C++ and Max generators.

Compiles an isolated C++ adapter, loads the actual Max core, and produces fixed-
seed observations. Metrics describe structure; none is a perceptual quality score.
No product files, current host state, or installed plugins are modified.
"""
import argparse
from collections import Counter
import csv
import gzip
import hashlib
import json
import math
from pathlib import Path
import statistics
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
STYLES = ['deep', 'rolling', 'offbeat', 'acid', 'disco', 'funk']
SOURCE_FILES = ['Source/PatternEngine.h', 'Source/PatternEngine.cpp', 'max/undertow-core.js']
METRICS = {
    'notes_per_bar': 'Note count divided by phrase length in four-beat bars.',
    'empty_bar_pct': 'Percentage of all phrase bars containing no note onset.',
    'bar_onset_jaccard': 'Intersection / union of bar-one/bar-two onset tick sets; ignores pitch and trailing duration.',
    'exact_bar_rhythm_pct': '100 if the two bars have identical onset tick sets, else 0.',
    'exact_bar_pitch_rhythm_pct': '100 if both bars have identical (onset tick, MIDI pitch) sets, else 0.',
    'pitch_classes': 'Number of distinct pitch classes in the phrase.',
    'pitch_range_semitones': 'Maximum MIDI pitch minus minimum MIDI pitch.',
    'root_note_pct': 'Percentage of notes with selected tonic pitch class.',
    'tonic_triad_note_pct': 'Percentage in C minor tonic triad C/Eb/G; a reference sonority, not known song harmony.',
    'strong_nontriad_pct': 'Percentage of quarter-beat onsets outside C/Eb/G; no claim these notes are necessarily wrong.',
    'nontriad_duration_pct': 'Percentage of total note duration outside C/Eb/G.',
    'long_nontriad_count': 'Number of notes outside C/Eb/G lasting >=0.5 beat.',
    'unresolved_nontriad_pct': 'Among nontriad notes, percentage lacking a following triad note 1–2 semitones away within 0.5 beat; descriptive neighbor-resolution rule.',
    'leap_over_fifth_pct': 'Percentage of cyclic adjacent-note intervals exceeding seven semitones.',
    'compound_leap_pct': 'Percentage of cyclic adjacent-note intervals exceeding twelve semitones.',
    'loop_boundary_leap': 'Absolute semitone difference between last and first MIDI pitch.',
    'second_bar_first_root_pct': '100 if first onset in second bar is tonic, else 0; 0 for empty bar.',
    'final_note_triad_pct': '100 if final onset is C/Eb/G, else 0.',
    'offbeat_eighth_pct': 'Percentage of onsets at half-beat positions between quarter pulses.',
    'sixteenth_pct': 'Percentage at odd sixteenth positions (including swing).',
    'variation_count_ratio': 'One same-control Variation count divided by original count.',
    'variation_new_onset_pitch_pct': 'Percentage of Variation notes with no identical onset tick and MIDI pitch in original.',
    'scale_violations': 'Count outside C natural minor; must remain zero for these unlocked settings.',
}
for lag in (1, 2):
    prefix = f'repeat_lag{lag}_'
    METRICS[prefix + 'onset_overlap_pct'] = (
        f'Across all bar pairs {lag} bars apart, 200 * shared onset ticks / total onsets in both sides. '
        'Null with no compared onsets. Bar-local onsets are rounded to 960 ticks/beat.')
    for dimension, detail in [('pitch', 'MIDI pitch'), ('duration', 'duration rounded to 960 ticks/beat'),
                              ('velocity', 'integer velocity'),
                              ('full_note', 'MIDI pitch, quantized duration, and integer velocity together')]:
        METRICS[prefix + dimension + '_retention_pct'] = (
            f'Among shared-onset note pairs {lag} bars apart, percentage with equal {detail}. '
            'Null when there are no shared onsets; read alongside onset overlap, not as whole-phrase similarity.')

DIVERSITY_METRICS = {}
for scope in ('first_bar', 'first_two_bar'):
    DIVERSITY_METRICS[f'distinct_{scope}_onset_motifs'] = (
        f'Number of distinct {scope.replace("_", " ")} onset signatures across seeds in this condition. '
        'Onsets are rounded to the nearest sixteenth; durations, velocities, and seed IDs are excluded.')
    DIVERSITY_METRICS[f'distinct_{scope}_pitch_onset_motifs'] = (
        f'Number of distinct {scope.replace("_", " ")} (nearest-sixteenth onset, MIDI pitch) signatures '
        'across seeds in this condition. Durations, velocities, and seed IDs are excluded.')
    DIVERSITY_METRICS[f'{scope}_pitch_onset_collision_pct'] = (
        f'Percentage of unordered pairs of different seed indices having identical {scope.replace("_", " ")} '
        'pitch/onset signatures. Null with fewer than two seeds. Describes this sample, not the possible output space.')

CPP_ADAPTER = r'''
#include "PatternEngine.h"
#include <iostream>
#include <iomanip>
#include <cstdlib>
void printPattern(const undertow::Pattern& p) {
    std::cout << "{\"lengthBeats\":" << p.lengthBeats << ",\"seed\":" << p.seed << ",\"notes\":[";
    for(int i=0;i<p.count;++i) { const auto& n=p.notes[i]; if(i)std::cout<<',';
        std::cout << "{\"start\":"<<n.start<<",\"duration\":"<<n.duration<<",\"pitch\":"<<n.pitch<<",\"velocity\":"<<n.velocity<<",\"locked\":false}"; }
    std::cout << "]}";
}
int main(int argc,char**argv) {
    const int count=argc>1?std::atoi(argv[1]):256,bars=argc>2?std::atoi(argv[2]):2;std::cout<<std::setprecision(17);
    for(int style=0;style<6;++style)for(int profile=0;profile<2;++profile)for(int i=0;i<count;++i) {
        undertow::Settings s;s.root=0;s.scale=undertow::Scale::minor;s.style=static_cast<undertow::Style>(style);
        s.bars=bars;s.octave=1;s.density=.55f;s.swing=.12f;s.gate=.65f;s.movement=profile?1.f:.35f;
        s.rootOnly=false;s.anchorRoot=true;s.octaveJumps=true;
        const std::uint32_t seed=0xa341316cu+static_cast<std::uint32_t>(i)*2654435761u;
        auto p=undertow::generate(s,seed);auto v=undertow::generate(s,seed+2654435769u,&p,true);
        std::cout<<"{\"engine\":\"cpp\",\"styleIndex\":"<<style<<",\"profileIndex\":"<<profile<<",\"seedIndex\":"<<i<<",\"pattern\":";
        printPattern(p);std::cout<<",\"variation\":";printPattern(v);
        if(i==0||i==63) {auto bytes=undertow::midiFile(p);std::cout<<",\"midi\":[";
            for(std::size_t j=0;j<bytes.size();++j){if(j)std::cout<<',';std::cout<<static_cast<int>(bytes[j]);}std::cout<<']';}
        std::cout<<"}\n";
    }
}
'''
JS_ADAPTER = r'''
const core=require(process.argv[1]);const count=Number(process.argv[2]),bars=Number(process.argv[3]);
for(let style=0;style<6;style++)for(let profile=0;profile<2;profile++)for(let i=0;i<count;i++) {
  const s={root:0,scale:0,style,bars,octave:1,density:.55,swing:.12,gate:.65,
    movement:profile?1:.35,rootOnly:false,anchorRoot:true,octaveJumps:true};
  const seed=(0xa341316c+Math.imul(i,2654435761))>>>0;
  const pattern=core.generate(s,seed),variation=core.generate(s,(seed+2654435769)>>>0,pattern,true);
  const row={engine:'max',styleIndex:style,profileIndex:profile,seedIndex:i,pattern,variation};
  if(i===0||i===63)row.midi=core.midiFile(pattern);console.log(JSON.stringify(row));
}
'''

def ratio(n, d):
    return n / d if d else 0.0

def tick(value):
    return math.floor(value * 960 + .5)

def describe_repetition(bars):
    result = {}
    for lag in (1, 2):
        total = shared = 0
        equal = dict.fromkeys(('pitch', 'duration', 'velocity', 'full_note'), 0)
        for b in range(lag, len(bars)):
            left = {tick(n['start'] - (b - lag) * 4): n for n in bars[b - lag]}
            right = {tick(n['start'] - b * 4): n for n in bars[b]}
            total += len(left) + len(right)
            common = left.keys() & right.keys()
            shared += len(common)
            for onset in common:
                a, z = left[onset], right[onset]
                same = {'pitch': a['pitch'] == z['pitch'],
                        'duration': tick(a['duration']) == tick(z['duration']),
                        'velocity': a['velocity'] == z['velocity']}
                for dimension in same:
                    equal[dimension] += same[dimension]
                equal['full_note'] += all(same.values())
        prefix = f'repeat_lag{lag}_'
        result[prefix + 'onset_overlap_pct'] = 200 * shared / total if total else None
        for dimension, count in equal.items():
            result[prefix + dimension + '_retention_pct'] = 100 * count / shared if shared else None
    return result


def describe_diversity(rows):
    result = {}
    for scope, bar_count in [('first_bar', 1), ('first_two_bar', 2)]:
        rhythm, pitched = Counter(), Counter()
        for row in rows:
            notes = [n for n in row['pattern']['notes'] if n['start'] < bar_count * 4]
            # A coarse structural signature prevents tiny timing/velocity differences
            # from masquerading as newly composed motifs. Pitch/register are retained.
            rhythm[tuple(sorted(math.floor(n['start'] * 4 + .5) for n in notes))] += 1
            pitched[tuple(sorted((math.floor(n['start'] * 4 + .5), n['pitch']) for n in notes))] += 1
        result[f'distinct_{scope}_onset_motifs'] = len(rhythm)
        result[f'distinct_{scope}_pitch_onset_motifs'] = len(pitched)
        pairs = len(rows) * (len(rows) - 1) // 2
        collisions = sum(n * (n - 1) // 2 for n in pitched.values())
        result[f'{scope}_pitch_onset_collision_pct'] = 100 * collisions / pairs if pairs else None
    return result


def describe(row):
    p, v = row['pattern'], row['variation']
    notes = p['notes']
    triad = {0, 3, 7}
    scale = {0, 2, 3, 5, 7, 8, 10}
    bar_count = int(p['lengthBeats'] / 4)
    bars = [[n for n in notes if b * 4 <= n['start'] < (b + 1) * 4] for b in range(bar_count)]
    # These existing fields intentionally retain their first-two-bars definition.
    onsets = [{tick(n['start'] - b * 4) for n in bars[b]} for b in range(2)]
    pitched = [{(tick(n['start'] - b * 4), n['pitch']) for n in bars[b]} for b in range(2)]
    strong = [n for n in notes if abs(n['start'] - round(n['start'])) < 1e-6]
    tensions = [n for n in notes if n['pitch'] % 12 not in triad]
    unresolved = 0
    leaps = []
    for i, n in enumerate(notes):
        nxt = notes[(i + 1) % len(notes)]
        interval = abs(nxt['pitch'] - n['pitch'])
        leaps.append(interval)
        gap = nxt['start'] - n['start'] + (p['lengthBeats'] if i == len(notes) - 1 else 0)
        if n['pitch'] % 12 not in triad:
            if not (nxt['pitch'] % 12 in triad and 1 <= interval <= 2 and gap <= .5 + 1e-6):
                unresolved += 1
    previous_note_set = {(tick(n['start']), n['pitch']) for n in notes}
    return {
        'notes_per_bar': len(notes) / bar_count,
        'empty_bar_pct': 100 * sum(not b for b in bars) / bar_count,
        'bar_onset_jaccard': ratio(len(onsets[0] & onsets[1]), len(onsets[0] | onsets[1])),
        'exact_bar_rhythm_pct': 100 * (onsets[0] == onsets[1]),
        'exact_bar_pitch_rhythm_pct': 100 * (pitched[0] == pitched[1]),
        'pitch_classes': len({n['pitch'] % 12 for n in notes}),
        'pitch_range_semitones': max(n['pitch'] for n in notes) - min(n['pitch'] for n in notes),
        'root_note_pct': 100 * ratio(sum(n['pitch'] % 12 == 0 for n in notes), len(notes)),
        'tonic_triad_note_pct': 100 * ratio(len(notes) - len(tensions), len(notes)),
        'strong_nontriad_pct': 100 * ratio(sum(n['pitch'] % 12 not in triad for n in strong), len(strong)) if strong else None,
        'nontriad_duration_pct': 100 * ratio(sum(n['duration'] for n in tensions), sum(n['duration'] for n in notes)),
        'long_nontriad_count': sum(n['duration'] >= .5 for n in tensions),
        'unresolved_nontriad_pct': 100 * ratio(unresolved, len(tensions)) if tensions else None,
        'leap_over_fifth_pct': 100 * ratio(sum(x > 7 for x in leaps), len(leaps)),
        'compound_leap_pct': 100 * ratio(sum(x > 12 for x in leaps), len(leaps)),
        'loop_boundary_leap': leaps[-1],
        'second_bar_first_root_pct': 100 * bool(bars[1] and bars[1][0]['pitch'] % 12 == 0),
        'final_note_triad_pct': 100 * (notes[-1]['pitch'] % 12 in triad),
        'offbeat_eighth_pct': 100 * ratio(sum(abs(n['start'] % 1 - .5) < 1e-6 for n in notes), len(notes)),
        'sixteenth_pct': 100 * ratio(sum(int(math.floor(n['start'] * 4 + .5)) % 2 for n in notes), len(notes)),
        'variation_count_ratio': ratio(len(v['notes']), len(notes)),
        'variation_new_onset_pitch_pct': 100 * ratio(sum((tick(n['start']), n['pitch']) not in previous_note_set for n in v['notes']), len(v['notes'])),
        'scale_violations': sum(n['pitch'] % 12 not in scale for n in notes),
        **describe_repetition(bars),
    }

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--seeds', type=int, default=256)
    parser.add_argument('--bars', type=int, choices=(2, 4, 8), default=2,
                        help='Phrase length; compare baselines only at the same length (default: 2).')
    args = parser.parse_args()
    if not 1 <= args.seeds <= 10000:
        parser.error('--seeds must be in 1..10000')
    out = args.output.resolve()
    if (out / 'manifest.json').exists():
        parser.error('output already contains an audit; choose a fresh output folder to preserve the baseline')
    out.mkdir(parents=True, exist_ok=True)
    (out / 'examples').mkdir(exist_ok=True)
    sources = {name: (ROOT / name).read_bytes() for name in SOURCE_FILES}
    with tempfile.TemporaryDirectory(prefix='undertow-audit-') as tmp:
        cpp = Path(tmp) / 'dump.cpp'
        exe = Path(tmp) / 'dump'
        cpp.write_text(CPP_ADAPTER)
        subprocess.run(['clang++', '-std=c++17', '-O2', '-I', str(ROOT / 'Source'), str(cpp),
                        str(ROOT / 'Source/PatternEngine.cpp'), '-o', str(exe)], check=True)
        raw_cpp = subprocess.run([str(exe), str(args.seeds), str(args.bars)], check=True, capture_output=True, text=True).stdout
    raw_js = subprocess.run(['node', '-e', JS_ADAPTER, str(ROOT / 'max/undertow-core.js'), str(args.seeds), str(args.bars)],
                            check=True, capture_output=True, text=True).stdout
    records = [json.loads(line) for line in (raw_cpp + raw_js).splitlines() if line]
    if len(records) != 24 * args.seeds:
        raise RuntimeError('Unexpected corpus size')
    for name, contents in sources.items():
        if (ROOT / name).read_bytes() != contents:
            raise RuntimeError('Source changed during audit: ' + name)
    metrics = []
    for row in records:
        row['style'] = STYLES[row.pop('styleIndex')]
        row['profile'] = ['default', 'movement-high'][row.pop('profileIndex')]
        description = describe(row)
        metrics.append({'engine': row['engine'], 'style': row['style'], 'profile': row['profile'],
                        'seed': row['pattern']['seed'], **description})
        if 'midi' in row:
            stem = '{}-{}-{}-seed{}'.format(row['engine'], row['style'], row['profile'], row['pattern']['seed'])
            (out / 'examples' / (stem + '.mid')).write_bytes(bytes(row.pop('midi')))
            (out / 'examples' / (stem + '.json')).write_text(json.dumps(row, indent=2) + '\n')
    with (out / 'patterns.csv').open('w', newline='') as f:
        writer = csv.DictWriter(f, fieldnames=list(metrics[0])); writer.writeheader(); writer.writerows(metrics)
    summaries = []
    for engine in ['cpp', 'max']:
        for style in STYLES:
            for profile in ['default', 'movement-high']:
                subset = [row for row in metrics if (row['engine'], row['style'], row['profile']) == (engine, style, profile)]
                summary = {'engine': engine, 'style': style, 'profile': profile, 'patterns': len(subset)}
                for metric in METRICS:
                    values = [row[metric] for row in subset if row[metric] is not None]
                    summary[metric] = statistics.mean(values) if values else None
                    summary[metric + '_sd'] = statistics.pstdev(values) if values else None
                    summary[metric + '_eligible_patterns'] = len(values)
                condition_records = [row for row in records
                                     if (row['engine'], row['style'], row['profile']) == (engine, style, profile)]
                summary.update(describe_diversity(condition_records))
                summaries.append(summary)
    with (out / 'summary.csv').open('w', newline='') as f:
        writer = csv.DictWriter(f, fieldnames=list(summaries[0])); writer.writeheader(); writer.writerows(summaries)
    corpus = ''.join(json.dumps(row, separators=(',', ':')) + '\n' for row in records).encode()
    (out / 'corpus.jsonl.gz').write_bytes(gzip.compress(corpus, mtime=0))
    for name, contents in sources.items():
        destination = out / 'source-snapshot' / name
        destination.parent.mkdir(parents=True, exist_ok=True); destination.write_bytes(contents)
    manifest = {
        'scope': 'Descriptive structural audit, not a listening test or musical-quality score.',
        'patterns': len(records), 'seeds_per_condition': args.seeds,
        'seed_formula': '(0xa341316c + index * 2654435761) mod 2^32, index=0..seeds-1',
        'settings': {'root': 0, 'scale': 'natural minor', 'bars': args.bars, 'octave': 1, 'density': .55,
                     'swing': .12, 'gate': .65, 'movement': [.35, 1], 'rootOnly': False,
                     'anchorRoot': True, 'octaveJumps': True},
        'variation_seed': '(base_seed + 2654435769) mod 2^32',
        'aggregation': 'Arithmetic mean of per-pattern metrics, with population standard deviation in *_sd fields. Conditional metrics are null when their denominator is empty and excluded from means; *_eligible_patterns records the denominator of each mean.',
        'tonic_triad_reference': 'C/Eb/G only. This is an explicit static C-minor test sonority, not the unknown track chords.',
        'source_sha256': {name: hashlib.sha256(contents).hexdigest() for name, contents in sources.items()},
        'corpus_sha256': hashlib.sha256(corpus).hexdigest(), 'metrics': METRICS,
        'cross_seed_metrics': DIVERSITY_METRICS,
        'comparison_limits': 'Use the same settings, bars, and seed count for comparisons. Diversity fields summarize '
                             'a condition across seeds; they are not means of per-pattern measurements. Repetition '
                             'and diversity are descriptive and do not establish musical quality or originality.',
    }
    (out / 'manifest.json').write_text(json.dumps(manifest, indent=2) + '\n')
    print(json.dumps({'output': str(out), 'patterns': len(records), 'summary_rows': len(summaries),
                      'examples': len(list((out / 'examples').glob('*.mid'))),
                      'scale_violations': sum(row['scale_violations'] for row in metrics)}, indent=2))

if __name__ == '__main__':
    main()
