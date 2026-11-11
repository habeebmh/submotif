#!/usr/bin/env python3
"""Compare actual C++ and Max composition, including stateful operations.

Requires Python 3, clang++ (or --compiler), and Node. Compiles only the portable
engine in an isolated temporary directory; does not build/install/package a
plugin. Control values are converted to IEEE float32 once at the comparison
boundary, matching C++ Settings. This tests edition parity, not musical taste.
"""
import argparse
import hashlib
import json
import math
from pathlib import Path
import struct
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
SOURCES = [ROOT / 'Source/PatternEngine.cpp', ROOT / 'Source/PatternEngine.h',
           ROOT / 'max/undertow-core.js']

CPP = r'''
#include "PatternEngine.h"
#include <iostream>
#include <iomanip>
using namespace undertow;
void emit(int id, int stage, const Pattern& p) {
    std::cout << "{\"id\":" << id << ",\"stage\":" << stage
      << ",\"pattern\":{\"lengthBeats\":" << p.lengthBeats
      << ",\"seed\":" << p.seed << ",\"notes\":[";
    for(int i=0; i<p.count; ++i) { const auto& n=p.notes[i]; if(i)std::cout<<',';
      std::cout << "{\"start\":" << n.start << ",\"duration\":" << n.duration
        << ",\"pitch\":" << n.pitch << ",\"velocity\":" << n.velocity
        << ",\"locked\":" << (n.locked ? "true" : "false") << '}'; }
    std::cout << "]}}\n";
}
int main() {
    std::cout << std::setprecision(17);
    int id, scale, style, rootOnly, anchorRoot, octaveJumps;
    std::uint32_t seed;
    Settings s;
    while(std::cin >> id >> seed >> s.root >> scale >> style >> s.bars >> s.octave
      >> s.density >> s.swing >> s.gate >> s.movement
      >> rootOnly >> anchorRoot >> octaveJumps) {
      s.scale=static_cast<Scale>(scale); s.style=static_cast<Style>(style);
      s.rootOnly=rootOnly!=0; s.anchorRoot=anchorRoot!=0; s.octaveJumps=octaveJumps!=0;
      auto p=generate(s,seed); emit(id,0,p);
      auto current=p;
      for(int round=1; round<=3; ++round) {
        current=generate(s,seed+static_cast<std::uint32_t>(round)*0x9e3779b9u,&current,true);
        emit(id,round,current);
      }
      auto locked=p;
      if(locked.count) { locked.notes[0].locked=true; locked.notes[locked.count-1].locked=true; }
      emit(id,4,generate(s,seed+11u,&locked,false));
      Settings changed=s;
      changed.root=(s.root+7)%12; changed.rootOnly=true; changed.octaveJumps=false;
      changed.octave=(s.octave+1)%3; changed.bars=s.bars==8?1:8;
      changed.swing=1.f-s.swing; changed.density=1.f; changed.gate=0.f;
      emit(id,5,generate(changed,seed+12u,&locked,true));
      Pattern manual; manual.lengthBeats=32.; manual.count=4; manual.seed=17;
      manual.notes[0]={.125,.2,127,1,true};
      manual.notes[1]={1.,.23,36,82,false};
      manual.notes[2]={3.625,.3,0,127,true};
      manual.notes[3]={31.5,.25,2,93,true};
      emit(id,6,generate(s,seed+13u,&manual,true));
    }
    return std::cin.eof()?0:2;
}
'''

JS = r'''
const fs=require('node:fs');
const core=require(process.argv[1]);
const rows=JSON.parse(fs.readFileSync(0,'utf8'));
function emit(id,stage,pattern){console.log(JSON.stringify({id,stage,pattern}));}
for(const row of rows) {
  const {id,seed,settings:s}=row;
  const p=core.generate(s,seed);emit(id,0,p);
  let current=p;
  for(let round=1;round<=3;round++) {
    current=core.generate(s,(seed+Math.imul(round,0x9e3779b9))>>>0,current,true);
    emit(id,round,current);
  }
  const locked=JSON.parse(JSON.stringify(p));
  if(locked.notes.length){locked.notes[0].locked=true;locked.notes[locked.notes.length-1].locked=true;}
  emit(id,4,core.generate(s,(seed+11)>>>0,locked,false));
  const changed=Object.assign({},s,{root:(s.root+7)%12,rootOnly:true,octaveJumps:false,
    octave:(s.octave+1)%3,bars:s.bars===8?1:8,swing:Math.fround(1-s.swing),density:1,gate:0});
  emit(id,5,core.generate(changed,(seed+12)>>>0,locked,true));
  const manual={lengthBeats:32,seed:17,notes:[
    {start:.125,duration:.2,pitch:127,velocity:1,locked:true},
    {start:1,duration:.23,pitch:36,velocity:82,locked:false},
    {start:3.625,duration:.3,pitch:0,velocity:127,locked:true},
    {start:31.5,duration:.25,pitch:2,velocity:93,locked:true}]};
  emit(id,6,core.generate(s,(seed+13)>>>0,manual,true));
}
'''

def float32(value):
    return struct.unpack('f', struct.pack('f', value))[0]

def scenarios():
    profiles = [
        dict(bars=2, density=.55, swing=.12, gate=.65, movement=.35,
             rootOnly=False, anchorRoot=True, octaveJumps=True),
        dict(bars=1, density=0., swing=0., gate=0., movement=0.,
             rootOnly=False, anchorRoot=False, octaveJumps=False),
        dict(bars=8, density=1., swing=1., gate=1., movement=1.,
             rootOnly=False, anchorRoot=False, octaveJumps=True),
        dict(bars=4, density=.73, swing=.33, gate=.42, movement=.85,
             rootOnly=True, anchorRoot=True, octaveJumps=True),
        dict(bars=2, density=.22, swing=.67, gate=.91, movement=1.,
             rootOnly=True, anchorRoot=False, octaveJumps=False),
        dict(bars=8, density=.91, swing=.01, gate=.17, movement=.67,
             rootOnly=False, anchorRoot=True, octaveJumps=False),
    ]
    seeds = [0, 1, 42, 0x80000000, 0xffffffff]
    result = []
    for style in range(6):
        for scale in range(5):
            for index, profile in enumerate(profiles):
                ident = len(result)
                s = dict(profile, style=style, scale=scale, root=(ident*17)%12,
                         octave=(style+scale+index)%3)
                for key in ('density','swing','gate','movement'):
                    s[key] = float32(s[key])
                result.append(dict(id=ident, seed=seeds[ident%len(seeds)], settings=s))
    return result

def input_line(row):
    s = row['settings']
    keys = ('root','scale','style','bars','octave','density','swing','gate','movement',
            'rootOnly','anchorRoot','octaveJumps')
    values = [row['id'], row['seed']] + [int(s[k]) if isinstance(s[k],bool) else s[k] for k in keys]
    return ' '.join(map(str,values))

def compare(left, right, tolerance):
    for key in ('lengthBeats','seed'):
        if left[key] != right[key]: return f'{key}: C++={left[key]}, Max={right[key]}'
    if len(left['notes']) != len(right['notes']):
        return f'note count: C++={len(left["notes"])}, Max={len(right["notes"])}'
    for i,(a,b) in enumerate(zip(left['notes'],right['notes'])):
        for key in ('start','duration','pitch','velocity','locked'):
            same = (abs(a[key]-b[key]) <= tolerance if key in ('start','duration')
                    and math.isfinite(a[key]) and math.isfinite(b[key]) else a[key] == b[key])
            if not same: return f'note {i} {key}: C++={a[key]}, Max={b[key]}'
    return None

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--compiler',default='clang++')
    parser.add_argument('--tolerance',type=float,default=1e-12,
                        help='Maximum absolute beat difference; pitches/velocity/locks/counts are exact.')
    args=parser.parse_args()
    if not math.isfinite(args.tolerance) or args.tolerance < 0:
        parser.error('--tolerance must be finite and nonnegative')
    before={str(p):hashlib.sha256(p.read_bytes()).hexdigest() for p in SOURCES}
    rows=scenarios()
    with tempfile.TemporaryDirectory(prefix='undertow-parity-') as temporary:
        cpp=Path(temporary)/'adapter.cpp'; exe=Path(temporary)/'adapter'
        cpp.write_text(CPP)
        subprocess.run([args.compiler,'-std=c++17','-O2','-I',str(ROOT/'Source'),str(cpp),
                        str(ROOT/'Source/PatternEngine.cpp'),'-o',str(exe)],check=True)
        cpp_result=subprocess.run([str(exe)],input='\n'.join(map(input_line,rows))+'\n',
                                  text=True,capture_output=True,check=True).stdout
    js_result=subprocess.run(['node','-e',JS,str(ROOT/'max/undertow-core.js')],
                             input=json.dumps(rows),text=True,capture_output=True,check=True).stdout
    after={str(p):hashlib.sha256(p.read_bytes()).hexdigest() for p in SOURCES}
    if before != after:
        raise RuntimeError('Engine source changed during comparison; rerun after edits are finished.')
    cpp_rows=[json.loads(line) for line in cpp_result.splitlines()]
    js_rows=[json.loads(line) for line in js_result.splitlines()]
    if len(cpp_rows)!=len(rows)*7 or len(js_rows)!=len(cpp_rows):
        raise RuntimeError('Unexpected output length; every scenario must execute all seven stages.')
    differences=[]
    for a,b in zip(cpp_rows,js_rows):
        if (a['id'],a['stage'])!=(b['id'],b['stage']):
            raise RuntimeError('Adapters disagreed on scenario/stage ordering.')
        diff=compare(a['pattern'],b['pattern'],args.tolerance)
        if diff: differences.append(f'case {a["id"]}, stage {a["stage"]}: {diff}')
    print(f'Compared {len(cpp_rows)} patterns across {len(rows)} scenarios, all six styles/five scales.')
    print('Stages: Generate, three consecutive Variations, regenerate with locks, changed controls with locks, off-grid/extreme-pitch locks.')
    if differences:
        for line in differences[:8]: print('FAIL '+line)
        print(f'{len(differences)} pattern mismatches; {len(cpp_rows)-len(differences)} matched.')
        return 1
    print(f'PASS: exact counts/pitches/velocities/locks/seeds; timing tolerance {args.tolerance:g} beat.')
    return 0

if __name__=='__main__':
    raise SystemExit(main())
