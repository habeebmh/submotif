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
