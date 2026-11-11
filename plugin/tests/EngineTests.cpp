#include "PatternEngine.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

using namespace undertow;
namespace {
int checks = 0;
void check(bool yes, const std::string& message) {
    ++checks;
    if (!yes) throw std::runtime_error(message);
}
bool sameNote(const Note& a, const Note& b) {
    return a.start == b.start && a.duration == b.duration && a.pitch == b.pitch
        && a.velocity == b.velocity && a.locked == b.locked;
}
bool samePattern(const Pattern& a, const Pattern& b) {
    if (a.count != b.count || a.lengthBeats != b.lengthBeats || a.seed != b.seed) return false;
    for (int i=0; i<a.count; ++i) if (!sameNote(a.notes[i], b.notes[i])) return false;
    return true;
}
bool samePlayback(const Pattern& a,const Pattern& b) {
    if(a.count!=b.count || a.lengthBeats!=b.lengthBeats)return false;
    for(int i=0;i<a.count;++i) {
        const auto& x=a.notes[i];const auto& y=b.notes[i];
        if(x.start!=y.start || x.duration!=y.duration || x.pitch!=y.pitch || x.velocity!=y.velocity)return false;
    }
    return true;
}
const std::array<std::vector<int>, 5> scales{{{0,2,3,5,7,8,10}, {0,2,3,5,7,9,10},
    {0,1,3,5,7,8,10}, {0,2,4,5,7,9,11}, {0,3,5,7,10}}};
bool inScale(int pitch, const Settings& s) {
    const int degree = (pitch-s.root+120)%12;
    const auto& scale = scales[static_cast<int>(s.scale)];
    return std::find(scale.begin(),scale.end(),degree)!=scale.end();
}
void generationTests() {
    Settings s;
    check(s.root == 0 && s.octave == 1 && s.bars == 1, "New phrases default to C1 and one bar");
    auto a = generate(s, 12345);
    check(a.count > 0, "generation must create playable notes");
    check(isValid(a), "default generated pattern must be valid");
    check(samePattern(a, generate(s,12345)), "same seed reproduces exact pattern");
    check(!samePlayback(a, generate(s,12346)), "fresh seed changes audible notes, not only metadata");
    Pattern empty;
    check(generate(s,12345,&empty,true).count>0, "Variation on an empty phrase generates playable notes");
    check(a.notes[0].pitch%12 == s.root, "root anchor applies to first sounding note");
    check(noteName(36)=="C1" && noteName(69)=="A3", "note names use Ableton octave convention");
    std::set<std::vector<int>> distinct;
    for(int style=0; style<6; ++style) {
        s.style=static_cast<Style>(style);
        for(int scale=0; scale<5; ++scale) {
            s.scale=static_cast<Scale>(scale);
            for(int seed=0; seed<120; ++seed) {
                s.root=seed%12;
                s.bars=std::array<int,4>{1,2,4,8}[seed%4];
                s.octave=seed%3;
                s.density=static_cast<float>(seed%11)/10.f;
                s.swing=static_cast<float>(seed%7)/6.f;
                s.gate=static_cast<float>(seed%9)/8.f;
                s.movement=static_cast<float>(seed%5)/4.f;
                s.rootOnly=seed%5==0;
                s.octaveJumps=seed%2==0;
                s.anchorRoot=seed%3!=0;
                auto p=generate(s,static_cast<std::uint32_t>(seed));
                check(isValid(p), "generated bounds/monophony across styles/seeds/settings");
                check(p.count>0 && p.count<=maxNotes, "bounded nonempty note count");
                check(p.lengthBeats==s.bars*4, "requested loop length");
                if(s.anchorRoot) check(p.notes[0].pitch%12==s.root, "root anchoring across styles");
                std::vector<int> signature;
                for(int n=0;n<p.count;++n) {
                    check(inScale(p.notes[n].pitch,s), "every generated pitch belongs to selected scale");
                    check(p.notes[n].pitch>=24 && p.notes[n].pitch<=71, "bass pitch range");
                    if(s.rootOnly) check(p.notes[n].pitch%12==s.root, "root-only pitch class");
                    if(s.rootOnly && !s.octaveJumps)
                        check(p.notes[n].pitch==(s.octave+2)*12+s.root, "no octaves in root-only with jumps off");
                    signature.push_back(p.notes[n].pitch);
                    signature.push_back(static_cast<int>(std::round(p.notes[n].start*960)));
                }
                distinct.insert(signature);
            }
        }
    }
    check(distinct.size()>1000, "settings and seeds provide broad pitch/rhythm diversity");
    s=Settings{}; s.style=Style::offbeat; s.anchorRoot=true; s.density=.4f; s.swing=0;
    int offbeat=0,total=0;
    for(int seed=0;seed<100;++seed) {
        auto p=generate(s,static_cast<std::uint32_t>(seed));
        for(int n=0;n<p.count;++n) {++total; if(std::abs(std::fmod(p.notes[n].start,1.0)-.5)<1.e-8) ++offbeat;}
    }
    check(offbeat*2>total, "offbeat style preferentially weights the offbeat eighths");
    s=Settings{}; s.bars=4;
    auto original=generate(s,8789);
    original.notes[1].locked=true;
    original.notes[original.count-1].locked=true;
    for(bool variation:{false,true}) {
        auto changed=generate(s,999,&original,variation);
        for(int i=0;i<original.count;++i) if(original.notes[i].locked) {
            bool found=false;
            for(int j=0;j<changed.count;++j) found|=sameNote(original.notes[i],changed.notes[j]);
            check(found,"locked note retained exactly through regeneration and variation");
        }
        check(isValid(changed),"locks must not create overlaps");
    }
    s.bars=1;
    check(isValid(generate(s,88,&original)),"shortened loop safely drops out-of-range locks");
    s=Settings{};
    auto base=generate(s,777);
    int retained=0;
    for(int seed=1;seed<=30;++seed) {
        auto varied=generate(s,seed,&base,true);
        for(int i=0;i<base.count;++i)
            for(int j=0;j<varied.count;++j)
                if(base.notes[i].pitch==varied.notes[j].pitch && base.notes[i].start==varied.notes[j].start) ++retained;
    }
    check(retained>base.count*10,"variation retains substantial rhythmic/pitch contour");
    // Unlocked variation material must obey a changed base register, just like
    // new notes. A locked note is the explicit exception to new controls.
    Pattern oldRegister; oldRegister.count=3; oldRegister.lengthBeats=8;
    oldRegister.notes[0]={0,.5,36,100,false};
    oldRegister.notes[1]={1,.5,36,100,false};
    oldRegister.notes[2]={2,.5,36,100,false};
    s=Settings{}; s.octave=2; s.rootOnly=true;
    for(bool jumps:{false,true}) {
        s.octaveJumps=jumps;
        for(int seed=1;seed<=20;++seed) {
            const auto raised=generate(s,seed,&oldRegister,true);
            for(int i=0;i<raised.count;++i) {
                check(raised.notes[i].pitch>=48 && raised.notes[i].pitch<=71,
                    "variation cannot retain unlocked pitches below new base or above bass range");
                if(!jumps) check(raised.notes[i].pitch==48,
                    "root-only variation with octaves off uses new base tonic exclusively");
            }
        }
    }
    oldRegister.notes[0].locked=true;
    s.octaveJumps=false;
    auto raisedWithLock=generate(s,1,&oldRegister,true);
    check(sameNote(raisedWithLock.notes[0],oldRegister.notes[0]),
        "locked old-register note remains an intentional exception");
    for(int i=1;i<raisedWithLock.count;++i)
        check(raisedWithLock.notes[i].pitch==48,"unlocked notes beside lock follow selected register");
    // A lock may leave a very short valid note immediately before it. Editing
    // articulation must not enlarge that note beyond the available gap.
    Pattern narrowGap; narrowGap.count=3; narrowGap.lengthBeats=4;
    narrowGap.notes[0]={0,.2,36,100,false};
    narrowGap.notes[1]={2,.002,36,100,false};
    narrowGap.notes[2]={2.003,.5,36,100,true};
    Settings narrowSettings; narrowSettings.rootOnly=true; narrowSettings.octaveJumps=false;
    check(isValid(narrowGap),"off-grid lock fixture is a valid phrase");
    const auto narrowVariation=generate(narrowSettings,5,&narrowGap,true);
    check(isValid(narrowVariation),"Variation articulation respects a close following lock");
    check(sameNote(narrowGap.notes[2],narrowVariation.notes[2]),"close following lock is exact");
    oldRegister.notes[0].locked=false;
    for(int i=0;i<oldRegister.count;++i) oldRegister.notes[i].pitch=84;
    s.octave=0; s.octaveJumps=true;
    for(int seed=1;seed<=20;++seed) {
        const auto lowered=generate(s,seed,&oldRegister,true);
        for(int i=0;i<lowered.count;++i)
            check(lowered.notes[i].pitch>=24 && lowered.notes[i].pitch<=71,
                "octave-enabled variation rejects unlocked notes above maximum bass register");
    }
    s.root=1000; s.bars=-1; s.octave=200; s.scale=static_cast<Scale>(999); s.style=static_cast<Style>(-40);
    s.density=std::numeric_limits<float>::quiet_NaN(); s.swing=std::numeric_limits<float>::infinity();
    s.gate=-20.f; s.movement=20.f;
    const auto safe=sanitise(s);
    check(safe.root>=0 && safe.root<12 && safe.bars==1 && safe.octave<=2,"invalid integer settings sanitized");
    check(std::isfinite(safe.density) && std::isfinite(safe.swing) && safe.gate>=0 && safe.movement<=1,"invalid floating settings sanitized");
    check(isValid(generate(s,3)),"invalid input cannot corrupt generation");
    Pattern invalid=base;
    invalid.count=maxNotes+1; check(!isValid(invalid),"reject oversized count");
    invalid=base; invalid.notes[0].duration=0; check(!isValid(invalid),"reject zero duration");
    invalid=base; invalid.notes[0].pitch=128; check(!isValid(invalid),"reject invalid pitch");
    invalid=base; invalid.notes[0].velocity=0; check(!isValid(invalid),"reject note-on zero velocity");
    invalid=base; invalid.notes[0].start=std::numeric_limits<double>::quiet_NaN(); check(!isValid(invalid),"reject nonfinite start");
    invalid=base; invalid.notes[1].start=invalid.notes[0].start; check(!isValid(invalid),"reject overlaps");
    for(double unsupportedLength:{std::numeric_limits<double>::denorm_min(),.001,1.,2.,12.,31.999999999}) {
        Pattern malformed; malformed.count=0; malformed.lengthBeats=unsupportedLength;
        check(!isValid(malformed),"reject loop lengths outside the supported 1/2/4/8 bars");
        // Keep validation first: the pre-fix denormal case would otherwise hang
        // in the scheduler when both cycle indexes become infinity.
        Scheduler player;
        const auto& events=player.process(malformed,1,120,48000,512,true,true,1);
        check(events.count==0,"invalid loop length returns safely without scheduling");
    }
    for(double supportedLength:{4.,8.,16.,32.}) {
        Pattern empty; empty.lengthBeats=supportedLength;
        check(isValid(empty),"supported empty patterns remain valid silent loops");
    }
}
bool isTonicTriad(int pitch,const Settings& s) {
    const int pc=(pitch-s.root+120)%12;
    return pc==0 || pc==7 || pc==(s.scale==Scale::major?4:3);
}
void musicalStructureTests() {
    int connectorCount=0,rollingNotes=0,rollingPhrases=0;
    for(int style=0;style<6;++style)for(int scale=0;scale<5;++scale)
    for(float movement:{0.f,.35f,1.f})for(int seed=0;seed<32;++seed) {
        Settings s;s.style=static_cast<Style>(style);s.scale=static_cast<Scale>(scale);
        s.movement=movement;s.bars=4;s.root=seed%12;s.octave=seed%3;
        const int base=(s.octave+2)*12+s.root;
        auto p=generate(s,0xa341316cu+static_cast<std::uint32_t>(seed)*2654435761u);
        check(isValid(p),"new phrase remains valid MIDI");
        for(int i=0;i<p.count;++i) {
            const auto& n=p.notes[i];
            check(n.pitch>=base && n.pitch<=std::min(base+12,71),"fresh phrases stay in a single octave register");
            const bool strong=std::abs(n.start-std::round(n.start))<1.e-6;
            if(strong)check(isTonicTriad(n.pitch,s),"strong attacks support the selected tonic triad");
            if(!isTonicTriad(n.pitch,s)) {
                ++connectorCount;
                const int step=static_cast<int>(std::floor(n.start*4+.5));
                check(step%2==1 && n.duration<=.20+1.e-8,"nontriad connectors are brief weak subdivisions");
                const auto& target=p.notes[(i+1)%p.count];
                const int interval=std::abs(target.pitch-n.pitch);
                const double gap=target.start-n.start+(i+1==p.count?p.lengthBeats:0);
                check(isTonicTriad(target.pitch,s) && interval>=1 && interval<=2 && gap<=.5+1.e-8,
                    "every fresh nontriad connector resolves stepwise to an actual nearby triad target including loop seam");
            }
        }
        // A phrase may answer near its ending, but its opening is a repeated
        // musical event sequence, including articulation and accent.
        std::vector<Note> opening;
        for(int i=0;i<p.count;++i)if(p.notes[i].start<2.0)opening.push_back(p.notes[i]);
        check(!opening.empty(),"motif supplies a recognizable opening");
        for(int bar=1;bar<4;++bar) {
            std::vector<Note> repeated;
            for(int i=0;i<p.count;++i)if(p.notes[i].start>=bar*4 && p.notes[i].start<bar*4+2) {
                auto n=p.notes[i];n.start-=bar*4;repeated.push_back(n);
            }
            check(repeated.size()==opening.size(),"all bars retain opening rhythm");
            for(std::size_t i=0;i<opening.size();++i)
                check(std::abs(repeated[i].start-opening[i].start)<1.e-8 && repeated[i].pitch==opening[i].pitch
                    && std::abs(repeated[i].duration-opening[i].duration)<1.e-8 && repeated[i].velocity==opening[i].velocity,
                    "repeated opening retains pitch, gate and accent");
        }
        if(style==1 && scale==0 && movement==.35f) {rollingNotes+=p.count;rollingPhrases+=4;}
        auto varied=p;
        const int initialCount=p.count;
        for(int pass=0;pass<5;++pass) {
            const auto prior=varied;
            varied=generate(s,static_cast<std::uint32_t>(987654+pass),&prior,true);
            check(isValid(varied) && varied.count==initialCount,"compatible Variation cannot accumulate notes");
            int changed=0;
            for(int i=0;i<prior.count;++i)if(!sameNote(prior.notes[i],varied.notes[i]))++changed;
            check(changed<=std::max(1,static_cast<int>(std::ceil(prior.count*.20))),"Variation edits a bounded subset");
            for(int i=0;i<varied.count;++i)if(std::fmod(varied.notes[i].start,4.0)<2.0)
                check(sameNote(varied.notes[i],prior.notes[i]),"Variation preserves the recognizable opening");
        }
    }
    check(connectorCount>0,"Movement can generate resolving scalar connectors");
    check(static_cast<double>(rollingNotes)/rollingPhrases>=5 && static_cast<double>(rollingNotes)/rollingPhrases<=9,
        "default Rolling has a controlled five-to-nine attack range on average");
    for(int style=0;style<6;++style) {
        Settings s;s.style=static_cast<Style>(style);s.density=1;s.movement=1;s.bars=8;
        const auto p=generate(s,222);
        check(p.count<=12*s.bars,"maximum density preserves rhythmic space");
    }
    // Distinguishing modal colors remain available as resolving approaches;
    // stronger harmonic support must not make the scale control inaudible.
    for(int style=0;style<6;++style)for(auto scale:{Scale::dorian,Scale::phrygian}) {
        Settings s;s.style=static_cast<Style>(style);s.scale=scale;s.movement=1;s.bars=1;
        bool heardCharacteristic=false;
        for(unsigned seed=0;seed<256;++seed) {
            const auto p=generate(s,0xa341316cu+seed*2654435761u);
            for(int i=0;i<p.count;++i)
                heardCharacteristic|=p.notes[i].pitch%12==(scale==Scale::dorian?9:1);
        }
        check(heardCharacteristic,"every style can voice the selected mode's characteristic color at high Movement");
    }
    std::array<double,6> durationSums{},weakCounts{},noteCounts{};
    for(int style=0;style<6;++style) {
        std::set<std::vector<int>> identities;
        for(unsigned seed=0;seed<128;++seed) {
            const auto id=0xa341316cu+seed*2654435761u;
            Settings s;s.style=static_cast<Style>(style);s.bars=1;s.movement=0;
            const auto phrase=generate(s,id);
            for(int i=0;i<phrase.count;++i) {
                const auto& n=phrase.notes[i];
                durationSums[style]+=n.duration;++noteCounts[style];
                const int step=static_cast<int>(std::floor(n.start*4+1.e-7));
                if(step%2)++weakCounts[style];
                if(style==2)check(step%4==2 && n.start+n.duration< std::floor(n.start)+1,
                    "unornamented Offbeat sustains its offbeat pulse and clears the following kick");
            }
            // At the same seed, Density adds to the generated identity; Gate
            // changes articulation without rerolling the rhythm or pitches.
            s.density=0;const auto sparse=generate(s,id);
            s.density=1;const auto dense=generate(s,id);
            check(dense.count>=sparse.count,"Density cannot remove attacks from the unornamented motif");
            for(int i=0;i<sparse.count;++i) {
                bool found=false;for(int j=0;j<dense.count;++j)found|=sparse.notes[i].start==dense.notes[j].start;
                check(found,"Density retains the stable rhythmic skeleton");
            }
            s.density=.55f;s.gate=0;const auto shortGates=generate(s,id);
            s.gate=1;const auto longGates=generate(s,id);
            check(shortGates.count==longGates.count,"Gate preserves note count");
            for(int i=0;i<shortGates.count;++i) {
                check(shortGates.notes[i].start==longGates.notes[i].start && shortGates.notes[i].pitch==longGates.notes[i].pitch,
                    "Gate preserves the generated rhythm and pitches");
                check(shortGates.notes[i].duration<=longGates.notes[i].duration+1.e-8,"Gate lengthens articulation within available space");
            }
            s.gate=.65f;s.movement=1;const auto developed=generate(s,id);
            std::vector<int> identity;
            for(int i=0;i<developed.count;++i) {
                identity.push_back(static_cast<int>(std::lround(developed.notes[i].start*960)));
                identity.push_back(developed.notes[i].pitch);
            }
            identities.insert(identity);
        }
        check(identities.size()>(style==2?3u:30u),"fixed controls retain fresh-seed pitch/rhythm diversity");
    }
    check(durationSums[0]/noteCounts[0]>durationSums[1]/noteCounts[1],"Deep leaves longer deliberate gates than Rolling");
    check(weakCounts[3]/noteCounts[3]>weakCounts[4]/noteCounts[4],"Acid has more sixteenth subdivision activity than Disco");
    check(weakCounts[5]/noteCounts[5]>weakCounts[4]/noteCounts[4],"Funk has more weak sixteenth attacks than Disco's eighth drive");
}
std::uint32_t be32(const std::vector<std::uint8_t>& bytes,std::size_t p) {
    return (std::uint32_t(bytes.at(p))<<24)|(std::uint32_t(bytes.at(p+1))<<16)|(std::uint32_t(bytes.at(p+2))<<8)|bytes.at(p+3);
}
struct ParsedMidi {
    std::uint32_t endTick=0;
    std::vector<std::array<int,4>> notes;
    bool meter=false, tempo=false, name=false;
};
ParsedMidi parseMidi(const std::vector<std::uint8_t>& bytes) {
    check(bytes.size()>=22,"MIDI has header and track");
    check(std::string(bytes.begin(),bytes.begin()+4)=="MThd" && be32(bytes,4)==6,"MIDI valid header");
    check(bytes[8]==0 && bytes[9]==0 && bytes[10]==0 && bytes[11]==1,"MIDI type0 single track");
    check(bytes[12]==3 && bytes[13]==192,"MIDI resolution is 960 PPQ");
    check(std::string(bytes.begin()+14,bytes.begin()+18)=="MTrk" && be32(bytes,18)==bytes.size()-22,"MIDI track byte length");
    std::size_t pos=22;
    std::uint32_t tick=0;
    auto vlq=[&]() {
        std::uint32_t value=0;
        int n=0;
        while(true) {
            auto b=bytes.at(pos++); value=(value<<7)|(b&127); ++n;
            check(n<=4,"MIDI variable length integer maximum 4 bytes");
            if(!(b&128)) break;
        }
        return value;
    };
    ParsedMidi out;
    while(pos<bytes.size()) {
        tick+=vlq(); auto status=bytes.at(pos++);
        if(status==255) {
            auto type=bytes.at(pos++); auto length=vlq();
            if(type==0x58) {out.meter=true; check(length==4 && bytes.at(pos)==4 && bytes.at(pos+1)==2,"4/4 meter metadata");}
            if(type==0x51) out.tempo=true;
            if(type==0x03) out.name=true;
            if(type==0x2f) {out.endTick=tick; check(length==0 && pos==bytes.size(),"explicit end-of-track at final event");}
            pos+=length;
        } else {
            check(status==0x90 || status==0x80,"only supported note channel events");
            auto pitch=bytes.at(pos++),velocity=bytes.at(pos++);
            out.notes.push_back({static_cast<int>(tick),status,pitch,velocity});
        }
    }
    return out;
}
void midiTests() {
    Pattern p; p.count=2; p.lengthBeats=8;
    p.notes[0]={.5,.5,36,99,false}; p.notes[1]={1.,.75,43,76,false};
    auto parsed=parseMidi(midiFile(p));
    check(parsed.meter && parsed.name && !parsed.tempo,"MIDI includes name/meter and leaves tempo to host");
    check(parsed.endTick==7680,"MIDI preserves trailing silence and loop length");
    check(parsed.notes.size()==4,"one MIDI on and off per pattern note");
    check(parsed.notes[0]==std::array<int,4>{480,0x90,36,99},"MIDI onset/pitch/velocity match pattern");
    check(parsed.notes[1]==std::array<int,4>{960,0x80,36,0},"note-off before simultaneous retrigger");
    check(parsed.notes[2]==std::array<int,4>{960,0x90,43,76},"simultaneous new note follows off");
    p.notes[0].pitch=128; check(midiFile(p).empty(),"invalid patterns cannot create corrupt MIDI");
    Settings s; s.bars=8; s.density=1; s.swing=1; s.gate=1;
    for(int seed=0;seed<50;++seed) {
        auto pattern=generate(s,seed); auto result=parseMidi(midiFile(pattern));
        check(result.endTick==30720,"eight-bar MIDI loop length");
        check(result.notes.size()==static_cast<std::size_t>(pattern.count*2),"every generated note exported");
        std::array<int,128> active{};
        for(auto e:result.notes) { active[e[2]]+=e[1]==0x90?1:-1; check(active[e[2]]>=0 && active[e[2]]<=1,"MIDI notes balanced and monophonic"); }
        check(std::all_of(active.begin(),active.end(),[](int n){return n==0;}),"export has no hanging notes");
    }
}
Pattern testPattern() {
    Pattern p; p.lengthBeats=4; p.count=3;
    p.notes[0]={0.,.5,36,100,false}; p.notes[1]={.5,.5,43,90,false}; p.notes[2]={3.5,.5,36,110,false};
    return p;
}
void schedulerTests() {
    auto p=testPattern(); Scheduler scheduler;
    auto e=scheduler.process(p,0,120,48000,12000,true,true,1);
    check(e.count==1 && e.events[0].on && e.events[0].sample==0 && e.events[0].pitch==36,"first onset at start, end-exclusive block");
    e=scheduler.process(p,.5,120,48000,12000,true,true,1);
    check(e.count==2 && !e.events[0].on && e.events[1].on && e.events[1].pitch==43,"boundary note-off before new onset");
    e=scheduler.process(p,1,120,48000,512,false,true,1);
    check(e.count==1 && !e.events[0].on && e.events[0].pitch==43,"stop releases only sounding note");
    e=scheduler.process(p,1,120,48000,512,false,true,1);
    check(e.count==0,"stopped blocks do not repeat stray note-offs");
    scheduler.reset();
    e=scheduler.process(p,.25,120,48000,1000,true,true,1);
    check(e.count==1 && e.events[0].on && e.events[0].sample==0,"start in middle of note chases note");
    e=scheduler.process(p,2,120,48000,1000,true,true,1);
    check(e.count==1 && !e.events[0].on,"seek into silence releases active note");
    e=scheduler.process(p,3.75,120,48000,1000,true,true,1);
    check(e.count==1 && e.events[0].on,"seek into note chases it");
    e=scheduler.process(p,0,120,48000,1000,true,true,1);
    check(e.count==2 && !e.events[0].on && e.events[1].on,"transport loop wrap releases and retriggers safely");
    e=scheduler.process(p,1000./24000,120,48000,1000,true,false,1);
    check(e.count==1 && !e.events[0].on,"disable flushes active note");
    e=scheduler.process(p,2000./24000,120,48000,1000,true,true,1);
    check(e.count==1 && e.events[0].on,"enable chases position");
    auto changed=p; changed.notes[0].pitch=40;
    e=scheduler.process(changed,3000./24000,120,48000,1000,true,true,2);
    check(e.count==2 && !e.events[0].on && e.events[0].pitch==36 && e.events[1].on && e.events[1].pitch==40,"pattern replacement releases old pitch");
    e=scheduler.process(changed,std::numeric_limits<double>::quiet_NaN(),120,48000,1000,true,true,2);
    check(e.count==1 && !e.events[0].on,"invalid timing safely releases notes");
    scheduler.reset();
    e=scheduler.process(p,-.25,120,48000,12000,true,true,1);
    check(e.count==3 && e.events[0].on && !e.events[1].on && e.events[2].on,"negative PPQ is normalized safely across zero");
    check(e.events[1].sample==6000 && e.events[2].sample==6000,"count-in zero boundary is sample accurate");

    // Event timelines must agree across host buffer partitioning, even with swing,
    // noninteger samples-per-beat, tempo change, and a note on the loop boundary.
    Settings s; s.bars=2; s.swing=.73f; s.gate=1.f; s.density=.8f;
    auto pattern=generate(s,986);
    auto timeline=[&](int blockSize) {
        Scheduler player; std::vector<std::array<int,4>> result;
        constexpr double bpm=127.3,rate=44100;
        const double beatPerSample=bpm/(60*rate);
        const int total=static_cast<int>(std::ceil(pattern.lengthBeats*3/beatPerSample));
        for(int pos=0;pos<total;) {
            int count=std::min(blockSize,total-pos);
            const auto& b=player.process(pattern,pos*beatPerSample,bpm,rate,count,true,true,1);
            for(int i=0;i<b.count;++i) {
                const auto& m=b.events[i];
                check(m.sample>=0 && m.sample<count,"event offset inside current audio buffer");
                result.push_back({pos+m.sample,m.pitch,m.velocity,m.on?1:0});
            }
            pos+=count;
        }
        const auto& stop=player.process(pattern,total*beatPerSample,bpm,rate,512,false,true,1);
        for(int i=0;i<stop.count;++i) result.push_back({total,stop.events[i].pitch,stop.events[i].velocity,0});
        return result;
    };
    const auto reference=timeline(64);
    check(reference==timeline(511) && reference==timeline(2048),"scheduler sample timeline independent of block size");
    std::array<int,128> active{};
    for(const auto& event:reference) {
        active[event[1]]+=event[3]?1:-1;
        check(active[event[1]]>=0 && active[event[1]]<=1,"no duplicate on or unmatched off in looping playback");
    }
    check(std::all_of(active.begin(),active.end(),[](int n){return n==0;}),"looping playback leaves no active notes after stop");
    // Changing tempo changes note scheduling without treating an uninterrupted
    // playhead as a seek or spuriously retriggering held notes.
    scheduler.reset();
    scheduler.process(p,0,120,48000,6000,true,true,1);
    e=scheduler.process(p,.25,90,48000,8000,true,true,1);
    check(e.count==0,"tempo change retains sounding note before its end");
    e=scheduler.process(p,.5,90,48000,512,true,true,1);
    check(e.count==2 && !e.events[0].on && e.events[1].on,"tempo changed boundary remains correct");
}
}
int main() {
    try {
        generationTests(); musicalStructureTests(); midiTests(); schedulerTests();
        std::cout << "PASS: " << checks << " engine checks (generation, scales, locks, MIDI export, transport).\n";
        return 0;
    } catch(const std::exception& e) {
        std::cerr << "FAIL after " << checks << " checks: " << e.what() << '\n';
        return 1;
    }
}
