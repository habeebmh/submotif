#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>
#include <iomanip>
#include <limits>
#include <locale>
#include <sstream>

namespace {
juce::String exactNumber(double number) {
    std::ostringstream stream;stream.imbue(std::locale::classic());
    stream << std::setprecision(std::numeric_limits<double>::max_digits10) << number;
    return stream.str();
}
double readExactNumber(const juce::XmlElement& xml,const char* name) {
    std::istringstream stream(xml.getStringAttribute(name).toStdString());
    stream.imbue(std::locale::classic());
    double number=0.0;
    if(!(stream>>number) || !stream.eof()) return std::numeric_limits<double>::quiet_NaN();
    return number;
}
bool sameSettings(const undertow::Settings& a, const undertow::Settings& b) {
    return a.root==b.root && a.scale==b.scale && a.style==b.style && a.bars==b.bars
        && a.octave==b.octave && a.density==b.density && a.swing==b.swing
        && a.gate==b.gate && a.movement==b.movement && a.rootOnly==b.rootOnly
        && a.anchorRoot==b.anchorRoot && a.octaveJumps==b.octaveJumps;
}
bool samePlayback(const undertow::Pattern& a,const undertow::Pattern& b) {
    if(a.count!=b.count || a.lengthBeats!=b.lengthBeats) return false;
    for(int i=0;i<a.count;++i) {
        const auto& n=a.notes[static_cast<size_t>(i)];const auto& m=b.notes[static_cast<size_t>(i)];
        if(n.start!=m.start || n.duration!=m.duration || n.pitch!=m.pitch || n.velocity!=m.velocity) return false;
    }
    return true;
}
std::unique_ptr<juce::XmlElement> writeSettings(const char* tag, const undertow::Settings& s) {
    auto x=std::make_unique<juce::XmlElement>(tag);
    x->setAttribute("root",s.root); x->setAttribute("scale",static_cast<int>(s.scale));
    x->setAttribute("style",static_cast<int>(s.style));x->setAttribute("bars",s.bars);
    x->setAttribute("octave",s.octave);x->setAttribute("density",static_cast<double>(s.density));
    x->setAttribute("swing",static_cast<double>(s.swing));x->setAttribute("gate",static_cast<double>(s.gate));
    x->setAttribute("movement",static_cast<double>(s.movement));x->setAttribute("rootOnly",s.rootOnly);
    x->setAttribute("anchorRoot",s.anchorRoot);x->setAttribute("octaveJumps",s.octaveJumps);
    return x;
}
undertow::Settings readSettings(const juce::XmlElement& x) {
    undertow::Settings s;
    s.root=x.getIntAttribute("root",0);s.scale=static_cast<undertow::Scale>(x.getIntAttribute("scale",0));
    s.style=static_cast<undertow::Style>(x.getIntAttribute("style",0));s.bars=x.getIntAttribute("bars",2);
    s.octave=x.getIntAttribute("octave",1);s.density=static_cast<float>(x.getDoubleAttribute("density",0.55));
    s.swing=static_cast<float>(x.getDoubleAttribute("swing",0.12));s.gate=static_cast<float>(x.getDoubleAttribute("gate",0.65));
    s.movement=static_cast<float>(x.getDoubleAttribute("movement",0.35));s.rootOnly=x.getBoolAttribute("rootOnly",false);
    s.anchorRoot=x.getBoolAttribute("anchorRoot",true);s.octaveJumps=x.getBoolAttribute("octaveJumps",true);
    return undertow::sanitise(s);
}
}

UndertowProcessor::UndertowProcessor()
    : AudioProcessor(BusesProperties().withOutput("Output",juce::AudioChannelSet::stereo(),true)) {
    pattern=undertow::generate(settings,static_cast<std::uint32_t>(random.nextInt()));
    committedSettings=settings;
    audioPattern=pattern;
}

bool UndertowProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const {
    return layouts.getMainInputChannelSet().isDisabled()
        && (layouts.getMainOutputChannelSet()==juce::AudioChannelSet::stereo()
            || layouts.getMainOutputChannelSet()==juce::AudioChannelSet::mono());
}
void UndertowProcessor::prepareToPlay(double sampleRate,int) {
    rate=std::isfinite(sampleRate) && sampleRate>0.0?sampleRate:44100.0;
}
void UndertowProcessor::releaseResources() { panicRequested.store(true); }
void UndertowProcessor::processBlock(juce::AudioBuffer<float>& buffer,juce::MidiBuffer& midi) { process(buffer,midi,false); }
void UndertowProcessor::processBlockBypassed(juce::AudioBuffer<float>& buffer,juce::MidiBuffer& midi) { process(buffer,midi,true); }

void UndertowProcessor::process(juce::AudioBuffer<float>& buffer,juce::MidiBuffer& midi,bool bypassed) {
    juce::ScopedNoDenormals noDenormals;
    buffer.clear();
    // The input bus exists for host compatibility, not musical passthrough.
    midi.clear();
    const auto requested=playbackRevision.load(std::memory_order_acquire);
    if(requested!=audioRevision) {
        const std::unique_lock<std::mutex> lock(stateMutex,std::try_to_lock);
        if(lock.owns_lock()) {
            audioPattern=pattern;
            const bool validShift=undertow::transposeOctaves(audioPattern,registerShift);
            jassert(validShift);juce::ignoreUnused(validShift);
            audioRevision=playbackRevision.load(std::memory_order_relaxed);
        }
    }
    double ppq=0.0, bpm=120.0;
    bool playing=false, positioned=false;
    if(const auto* playHead=getPlayHead()) {
        if(const auto pos=playHead->getPosition()) {
            if(const auto tempo=pos->getBpm();tempo && std::isfinite(*tempo) && *tempo>0.0) bpm=*tempo;
            if(const auto beat=pos->getPpqPosition();beat && std::isfinite(*beat)) {
                ppq=*beat;positioned=true;playing=pos->getIsPlaying();
            }
        }
    }
    hostBpm.store(bpm,std::memory_order_relaxed);
    hostPlaying.store(playing,std::memory_order_relaxed);
    hostPosition.store(positioned,std::memory_order_relaxed);
    const bool outputEnabled=enabled.load(std::memory_order_relaxed) && !bypassed;
    const bool panicNow=panicRequested.exchange(false);
    // A panic/prepare block flushes old notes; the next host block chases correctly.
    const auto& events=scheduler.process(audioPattern,ppq,bpm,rate,buffer.getNumSamples(),
                                        playing,outputEnabled && !panicNow,audioRevision);
    for(int i=0;i<events.count;++i) {
        const auto& e=events.events[static_cast<size_t>(i)];
        midi.addEvent(e.on ? juce::MidiMessage::noteOn(1,e.pitch,static_cast<juce::uint8>(e.velocity))
                           : juce::MidiMessage::noteOff(1,e.pitch),e.sample);
    }
    double phase=std::fmod(ppq,audioPattern.lengthBeats);
    if(phase<0.0) phase+=audioPattern.lengthBeats;
    playheadBeat.store(playing && outputEnabled ? phase : -1.0,std::memory_order_relaxed);
}

undertow::Settings UndertowProcessor::getSettings() const { const std::lock_guard<std::mutex> lock(stateMutex);return settings; }
undertow::Pattern UndertowProcessor::getPattern() const {
    const std::lock_guard<std::mutex> lock(stateMutex);
    auto output=pattern;const bool validShift=undertow::transposeOctaves(output,registerShift);
    jassert(validShift);juce::ignoreUnused(validShift);return output;
}
int UndertowProcessor::getRegisterShift() const { const std::lock_guard<std::mutex> lock(stateMutex);return registerShift; }
bool UndertowProcessor::canSetRegisterShift(int octaves) const {
    const std::lock_guard<std::mutex> lock(stateMutex);
    auto output=pattern;return undertow::transposeOctaves(output,octaves);
}
bool UndertowProcessor::setRegisterShift(int octaves) {
    {
        const std::lock_guard<std::mutex> lock(stateMutex);
        auto output=pattern;if(!undertow::transposeOctaves(output,octaves)) return false;
        if(octaves==registerShift) return true;
        remember();commitPattern(pattern,octaves);
    }
    updateHostDisplay(ChangeDetails().withNonParameterStateChanged(true));return true;
}
bool UndertowProcessor::hasPendingSettings() const { const std::lock_guard<std::mutex> lock(stateMutex);return !sameSettings(settings,committedSettings); }
void UndertowProcessor::setSettings(undertow::Settings newSettings) {
    {const std::lock_guard<std::mutex> lock(stateMutex);settings=undertow::sanitise(newSettings);}
    updateHostDisplay(ChangeDetails().withNonParameterStateChanged(true));
}
void UndertowProcessor::remember() {
    undoStack.push_back({settings,committedSettings,pattern,registerShift});
    if(undoStack.size()>64) undoStack.pop_front();
    redoStack.clear();
}
void UndertowProcessor::commitPattern(const undertow::Pattern& newPattern,int shift) {
    const bool playbackChanged=registerShift!=shift || !samePlayback(pattern,newPattern);
    pattern=newPattern;registerShift=shift;
    revision.fetch_add(1,std::memory_order_release);
    if(playbackChanged) playbackRevision.fetch_add(1,std::memory_order_release);
}
bool UndertowProcessor::generateNew(bool variation) {
    {
        const std::lock_guard<std::mutex> lock(stateMutex);
        auto seed=static_cast<std::uint32_t>(random.nextInt());
        if(seed==pattern.seed) ++seed;
        // A retained motif cannot express new rhythm/gate/style settings.
        const auto next=undertow::generate(settings,seed,&pattern,
            variation && sameSettings(settings,committedSettings));
        auto output=next;
        if(!undertow::transposeOctaves(output,registerShift)) return false;
        remember();committedSettings=settings;commitPattern(next,registerShift);
    }
    updateHostDisplay(ChangeDetails().withNonParameterStateChanged(true));return true;
}
void UndertowProcessor::toggleLock(int index) {
    {
        const std::lock_guard<std::mutex> lock(stateMutex);
        if(index<0 || index>=pattern.count) return;
        remember();pattern.notes[static_cast<size_t>(index)].locked=!pattern.notes[static_cast<size_t>(index)].locked;
        revision.fetch_add(1,std::memory_order_release);
    }
    updateHostDisplay(ChangeDetails().withNonParameterStateChanged(true));
}
void UndertowProcessor::clearLocks() {
    {
        const std::lock_guard<std::mutex> lock(stateMutex);
        bool any=false;for(int i=0;i<pattern.count;++i) any|=pattern.notes[static_cast<size_t>(i)].locked;
        if(!any) return;
        remember();for(int i=0;i<pattern.count;++i) pattern.notes[static_cast<size_t>(i)].locked=false;
        revision.fetch_add(1,std::memory_order_release);
    }
    updateHostDisplay(ChangeDetails().withNonParameterStateChanged(true));
}
void UndertowProcessor::undo() {
    {
        const std::lock_guard<std::mutex> lock(stateMutex);if(undoStack.empty()) return;
        redoStack.push_back({settings,committedSettings,pattern,registerShift});
        const auto snapshot=undoStack.back();undoStack.pop_back();
        settings=snapshot.settings;committedSettings=snapshot.committedSettings;commitPattern(snapshot.pattern,snapshot.registerShift);
    }
    updateHostDisplay(ChangeDetails().withNonParameterStateChanged(true));
}
void UndertowProcessor::redo() {
    {
        const std::lock_guard<std::mutex> lock(stateMutex);if(redoStack.empty()) return;
        undoStack.push_back({settings,committedSettings,pattern,registerShift});
        const auto snapshot=redoStack.back();redoStack.pop_back();
        settings=snapshot.settings;committedSettings=snapshot.committedSettings;commitPattern(snapshot.pattern,snapshot.registerShift);
    }
    updateHostDisplay(ChangeDetails().withNonParameterStateChanged(true));
}
bool UndertowProcessor::canUndo() const { const std::lock_guard<std::mutex> lock(stateMutex);return !undoStack.empty(); }
bool UndertowProcessor::canRedo() const { const std::lock_guard<std::mutex> lock(stateMutex);return !redoStack.empty(); }
void UndertowProcessor::setEnabled(bool value) { enabled.store(value);updateHostDisplay(ChangeDetails().withNonParameterStateChanged(true)); }

void UndertowProcessor::getStateInformation(juce::MemoryBlock& data) {
    juce::XmlElement state("UNDERTOW");
    {
        const std::lock_guard<std::mutex> lock(stateMutex);
        state.setAttribute("version",1);state.setAttribute("enabled",enabled.load());
        state.setAttribute("registerShift",registerShift);
        state.addChildElement(writeSettings("Settings",settings).release());
        state.addChildElement(writeSettings("CommittedSettings",committedSettings).release());
        auto* p=state.createNewChildElement("Pattern");
        p->setAttribute("seed",juce::String(static_cast<juce::int64>(pattern.seed)));
        p->setAttribute("length",pattern.lengthBeats);
        for(int i=0;i<pattern.count;++i) {
            auto* n=p->createNewChildElement("Note");const auto& note=pattern.notes[static_cast<size_t>(i)];
            n->setAttribute("start",exactNumber(note.start));n->setAttribute("duration",exactNumber(note.duration));
            n->setAttribute("pitch",note.pitch);n->setAttribute("velocity",note.velocity);n->setAttribute("locked",note.locked);
        }
    }
    copyXmlToBinary(state,data);
}
void UndertowProcessor::setStateInformation(const void* data,int size) {
    if(data==nullptr || size<=0 || size>1024*1024) return;
    auto state=getXmlFromBinary(data,size);
    if(!state || !state->hasTagName("UNDERTOW") || state->getIntAttribute("version")!=1) return;
    const auto* s=state->getChildByName("Settings");const auto* p=state->getChildByName("Pattern");
    const auto* c=state->getChildByName("CommittedSettings");
    if(!s || !p || !c || p->getNumChildElements()>undertow::maxNotes) return;
    undertow::Pattern incoming;
    incoming.lengthBeats=p->getDoubleAttribute("length",0.0);
    incoming.seed=static_cast<std::uint32_t>(p->getStringAttribute("seed").getLargeIntValue());
    for(const auto* n:p->getChildIterator()) {
        if(!n->hasTagName("Note")) return;
        auto& note=incoming.notes[static_cast<size_t>(incoming.count++)];
        note.start=readExactNumber(*n,"start");note.duration=readExactNumber(*n,"duration");
        note.pitch=n->getIntAttribute("pitch",-1);note.velocity=n->getIntAttribute("velocity",0);note.locked=n->getBoolAttribute("locked",false);
    }
    const double savedShift=state->hasAttribute("registerShift") ? readExactNumber(*state,"registerShift") : 0.0;
    if(!std::isfinite(savedShift)||savedShift!=std::floor(savedShift)||savedShift < -5||savedShift > 5) return;
    const int incomingShift=static_cast<int>(savedShift);
    auto output=incoming;if(!undertow::transposeOctaves(output,incomingShift)) return;
    {
        const std::lock_guard<std::mutex> lock(stateMutex);
        settings=readSettings(*s);committedSettings=readSettings(*c);
        undoStack.clear();redoStack.clear();commitPattern(incoming,incomingShift);
        enabled.store(state->getBoolAttribute("enabled",true));
    }
}

juce::File UndertowProcessor::createMidiDragFile() {
    auto directory=juce::File::getSpecialLocation(juce::File::tempDirectory).getChildFile("Submotif-MIDI");
    if(!directory.createDirectory().wasOk()) return {};
    const auto name="Submotif-"+juce::Uuid().toString()+".mid";
    auto file=directory.getChildFile(name);
    return exportMidiTo(file)?file:juce::File{};
}
bool UndertowProcessor::exportMidiTo(const juce::File& file) {
    const auto snapshot=getPattern();if(!undertow::isValid(snapshot)) return false;
    const auto bytes=undertow::midiFile(snapshot);if(bytes.empty()) return false;
    juce::TemporaryFile temporary(file);
    { auto stream=temporary.getFile().createOutputStream();
      if(!stream || !stream->write(bytes.data(),bytes.size())) return false;
      stream->flush();if(stream->getStatus().failed()) return false; }
    return temporary.overwriteTargetFileWithTemporary();
}
juce::AudioProcessorEditor* UndertowProcessor::createEditor() { return new UndertowEditor(*this); }
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new UndertowProcessor(); }
