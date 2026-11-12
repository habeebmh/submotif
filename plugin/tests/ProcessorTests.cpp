#include "PluginProcessor.h"
#include <juce_audio_utils/juce_audio_utils.h>
#include <iostream>
#include <stdexcept>
#include <set>
#include <iomanip>

namespace {
void require(bool ok, const char* message) { if (!ok) throw std::runtime_error(message); }
class TestPlayHead final : public juce::AudioPlayHead {
public:
    PositionInfo position;
    juce::Optional<PositionInfo> getPosition() const override { return position; }
};
bool sameNotes(const undertow::Pattern& a, const undertow::Pattern& b) {
    if(a.count != b.count || a.lengthBeats != b.lengthBeats || a.seed != b.seed) return false;
    for(int i=0;i<a.count;++i) {
        auto& n=a.notes[static_cast<size_t>(i)]; auto& m=b.notes[static_cast<size_t>(i)];
        if(n.start!=m.start || n.duration!=m.duration || n.pitch!=m.pitch || n.velocity!=m.velocity || n.locked!=m.locked) {
            std::cerr << std::setprecision(17) << "Note mismatch " << i << ": start " << n.start << '/' << m.start
                      << " duration " << n.duration << '/' << m.duration << " pitch " << n.pitch << '/' << m.pitch << '\n';
            return false;
        }
    }
    return true;
}
std::vector<unsigned char> midiBytes(const juce::MidiBuffer& buffer) {
    std::vector<unsigned char> bytes;
    for (const auto event : buffer) {
        const auto message = event.getMessage();
        for (int shift = 0; shift < 32; shift += 8)
            bytes.push_back(static_cast<unsigned char>(event.samplePosition >> shift));
        bytes.push_back(static_cast<unsigned char>(message.getRawDataSize()));
        bytes.insert(bytes.end(), message.getRawData(), message.getRawData() + message.getRawDataSize());
    }
    return bytes;
}
void checkRegisterShift() {
    UndertowProcessor p;
    auto settings=p.getSettings();settings.rootOnly=true;settings.octaveJumps=false;
    p.setSettings(settings);require(p.generateNew(),"generate at default register");p.toggleLock(0);
    const auto original=p.getPattern();
    require(p.getRegisterShift()==0,"new devices use original C1 register");
    for(int shift : {5,2,-3,0}) {
        require(p.setRegisterShift(shift),"valid register change succeeds");
        auto expected=original;require(undertow::transposeOctaves(expected,shift),"expected transposition is valid");
        require(sameNotes(expected,p.getPattern()),"register immediately shifts all notes including locks without accumulating offsets");
    }
    const auto revision=p.getRevision();
    require(!p.setRegisterShift(-4)&&!p.setRegisterShift(6),"invalid register changes fail");
    require(p.getRevision()==revision&&sameNotes(original,p.getPattern()),"rejection leaves the phrase unchanged");
    require(p.setRegisterShift(5),"raise five octaves");p.undo();
    require(p.getRegisterShift()==0&&sameNotes(original,p.getPattern()),"undo restores register and exact notes");p.redo();
    require(p.getRegisterShift()==5,"redo restores register");
    settings.density=.2f;p.setSettings(settings);require(p.hasPendingSettings(),"independent generation settings remain pending");
    require(p.setRegisterShift(4)&&p.hasPendingSettings(),"transposition must not apply unrelated pending settings");
    require(p.generateNew()&&p.getRegisterShift()==4&&!p.hasPendingSettings(),"Generate retains the chosen shift");
    for(int i=0;i<p.getPattern().count;++i)require(p.getPattern().notes[i].pitch==84,"new notes use shifted C5 register");
    juce::MemoryBlock saved;p.getStateInformation(saved);UndertowProcessor restored;
    restored.setStateInformation(saved.getData(),static_cast<int>(saved.getSize()));
    require(restored.getRegisterShift()==4&&sameNotes(restored.getPattern(),p.getPattern()),"project state retains shifted notes exactly");
    auto xml=juce::AudioProcessor::getXmlFromBinary(saved.getData(),static_cast<int>(saved.getSize()));
    xml->setAttribute("registerShift",6);juce::MemoryBlock invalid;juce::AudioProcessor::copyXmlToBinary(*xml,invalid);
    restored.setStateInformation(invalid.getData(),static_cast<int>(invalid.getSize()));
    require(restored.getRegisterShift()==4,"invalid saved shift does not replace current state");
    xml->removeAttribute("registerShift");juce::AudioProcessor::copyXmlToBinary(*xml,invalid);
    restored.setStateInformation(invalid.getData(),static_cast<int>(invalid.getSize()));
    require(restored.getRegisterShift()==0&&restored.getPattern().notes[0].pitch==36,"older projects without shift keep their original register");

    // A held note must release at its old pitch before the shifted note sounds.
    p.setRegisterShift(0);TestPlayHead host;host.position.setBpm(120);host.position.setIsPlaying(true);
    const auto first=p.getPattern().notes[0];host.position.setPpqPosition(first.start+.001);
    p.setPlayHead(&host);p.prepareToPlay(48000,16);juce::AudioBuffer<float> audio(2,16);juce::MidiBuffer events;
    p.processBlock(audio,events);require(!events.isEmpty(),"initial held note sounds");
    p.setRegisterShift(2);host.position.setPpqPosition(first.start+.002);events.clear();p.processBlock(audio,events);
    bool off=false,on=false;
    for(const auto event:events) {
        auto m=event.getMessage();if(m.isNoteOff()&&m.getNoteNumber()==first.pitch)off=true;
        if(m.isNoteOn()){require(off,"old note released before shifted attack");require(m.getNoteNumber()==first.pitch+24,"playback uses shifted pitch");on=true;}
    }
    require(off&&on,"changing register releases and chases the held note");
    const auto file=p.createMidiDragFile();auto input=file.createInputStream();juce::MidiFile midi;
    require(input&&midi.readFrom(*input),"transposed MIDI export parses");int notes=0;
    for(int i=0;i<midi.getTrack(0)->getNumEvents();++i){const auto& m=midi.getTrack(0)->getEventPointer(i)->message;if(m.isNoteOn()){require(m.getNoteNumber()==60,"export matches shifted playback");++notes;}}
    require(notes==p.getPattern().count,"export retains all transposed notes");file.deleteFile();
}

void checkInputIsolation() {
    UndertowProcessor clean, fed;
    juce::MemoryBlock state; clean.getStateInformation(state);
    fed.setStateInformation(state.getData(), static_cast<int>(state.getSize()));
    TestPlayHead host;
    host.position.setBpm(120.0); host.position.setIsPlaying(true);
    clean.setPlayHead(&host); fed.setPlayHead(&host);
    clean.prepareToPlay(48000, 512); fed.prepareToPlay(48000, 512);
    juce::AudioBuffer<float> cleanAudio(2, 512), fedAudio(2, 512);
    int generated = 0;
    for (int block = 0; block < 360; ++block) {
        const bool disabled = block >= 240 && block < 280;
        const bool bypassed = block >= 280 && block < 320;
        clean.setEnabled(!disabled); fed.setEnabled(!disabled);
        host.position.setIsPlaying(block < 340);
        host.position.setPpqPosition(static_cast<double>(block) * 512.0 / 24000.0);
        juce::MidiBuffer cleanMidi, fedMidi;
        // Include notes on the generator's own channel/pitch as well as foreign
        // channels and non-note messages; exact comparison detects any echo.
        fedMidi.addEvent(juce::MidiMessage::noteOn(1, 36, static_cast<juce::uint8>(127)), 0);
        fedMidi.addEvent(juce::MidiMessage::noteOn(7, 105, static_cast<juce::uint8>(127)), 3);
        fedMidi.addEvent(juce::MidiMessage::controllerEvent(1, 64, 127), 8);
        fedMidi.addEvent(juce::MidiMessage::pitchWheel(7, 1234), 16);
        fedMidi.addEvent(juce::MidiMessage::noteOff(1, 36), 31);
        fedMidi.addEvent(juce::MidiMessage::noteOff(7, 105), 32);
        if (bypassed) {
            clean.processBlockBypassed(cleanAudio, cleanMidi); fed.processBlockBypassed(fedAudio, fedMidi);
        } else {
            clean.processBlock(cleanAudio, cleanMidi); fed.processBlock(fedAudio, fedMidi);
        }
        require(midiBytes(cleanMidi) == midiBytes(fedMidi),
                "External MIDI cannot alter or pass through generated, disabled, bypassed or stopped output");
        for (const auto event : cleanMidi) if (event.getMessage().isNoteOn()) ++generated;
    }
    require(generated > 0, "Input filtering preserves the generator's own output");
    juce::AudioBuffer<float> zero(2, 0); juce::MidiBuffer events;
    events.addEvent(juce::MidiMessage::noteOn(7, 105, static_cast<juce::uint8>(127)), 0);
    fed.processBlock(zero, events);
    require(events.isEmpty(), "Direct zero-sample callback discards input MIDI");
}
}
int main(int argc, char* argv[]) {
    juce::ScopedJuceInitialiser_GUI init;
    try {
        UndertowProcessor p;
        require(p.acceptsMidi() && p.producesMidi(), "Generator advertises Live-compatible MIDI input and output buses");
        checkRegisterShift();
        checkInputIsolation();
        require(undertow::isValid(p.getPattern()), "Initial pattern must be valid");
        p.toggleLock(0);
        const auto before = p.getPattern();
        p.generateNew();
        const auto after = p.getPattern();
        require(before.seed != after.seed, "Each generate must use a fresh seed");
        require(p.canUndo(), "Generate must create undo history");
        p.undo();
        require(sameNotes(before,p.getPattern()), "Undo restores exact pattern");
        p.redo();
        require(sameNotes(after,p.getPattern()), "Redo restores exact pattern");
        auto settings = p.getSettings(); settings.rootOnly=true; settings.style=undertow::Style::disco;
        p.setSettings(settings);
        require(p.hasPendingSettings(), "Pending controls must be reported");
        juce::MemoryBlock state;
        p.getStateInformation(state);
        UndertowProcessor restored;
        restored.setStateInformation(state.getData(),static_cast<int>(state.getSize()));
        require(sameNotes(p.getPattern(),restored.getPattern()), "State roundtrip preserves exact notes, seed and locks");
        require(restored.getSettings().rootOnly && restored.getSettings().style==undertow::Style::disco, "State preserves settings");
        require(restored.hasPendingSettings(), "State preserves pending versus committed settings");
        restored.setStateInformation("corrupt",7);
        require(sameNotes(p.getPattern(),restored.getPattern()), "Malformed state must leave valid state intact");
        restored.setStateInformation(state.getData(),10);
        require(sameNotes(p.getPattern(),restored.getPattern()), "Truncated state must leave valid state intact");
        restored.generateNew();
        require(!restored.hasPendingSettings(), "Generate commits settings");

        // A Variation click must apply pending controls, rather than marking
        // them committed while leaving the old density/gates unchanged.
        UndertowProcessor controlChange;
        auto sparse=controlChange.getSettings();
        sparse.style=undertow::Style::offbeat;sparse.density=0;sparse.gate=0;
        sparse.rootOnly=true;sparse.octaveJumps=false;sparse.movement=0;
        controlChange.setSettings(sparse);controlChange.generateNew();controlChange.toggleLock(0);
        const auto sparsePattern=controlChange.getPattern();
        auto dense=sparse;dense.density=1;dense.gate=1;
        controlChange.setSettings(dense);controlChange.generateNew(true);
        const auto densePattern=controlChange.getPattern();
        require(densePattern.count>sparsePattern.count,"Variation applies changed Density");
        bool longer=false,keptLock=false;
        for(int i=0;i<densePattern.count;++i) {
            const auto& n=densePattern.notes[i];
            if(!n.locked && n.duration>.3) longer=true;
            const auto& locked=sparsePattern.notes[0];
            if(n.locked && n.start==locked.start && n.duration==locked.duration
               && n.pitch==locked.pitch && n.velocity==locked.velocity) keptLock=true;
        }
        require(longer,"Variation applies changed Gate");
        require(keptLock,"Applying controls through Variation preserves explicit locks");
        require(!controlChange.hasPendingSettings(),"Variation commits applied controls");

        auto file = p.createMidiDragFile();
        require(file.existsAsFile(), "Drag creates readable real MIDI file");
        auto input = file.createInputStream(); juce::MidiFile midi;
        require(input && midi.readFrom(*input), "Export must parse with independent JUCE MIDI reader");
        require(midi.getTimeFormat()==960 && midi.getNumTracks()==1, "MIDI type zero / 960 PPQ");
        auto* track=midi.getTrack(0); int noteOns=0;
        for (int i=0;i<track->getNumEvents();++i) {
            const auto& m=track->getEventPointer(i)->message;
            if(m.isNoteOn()) ++noteOns;
            require(!m.isTempoMetaEvent(), "Export must not override project tempo");
        }
        require(noteOns==p.getPattern().count, "Export contains every committed note");
        require(track->getEndTime()==p.getPattern().lengthBeats*960.0, "Export retains full loop length");

        TestPlayHead host; p.setPlayHead(&host); p.prepareToPlay(48000,512);
        host.position.setBpm(120.0); host.position.setIsPlaying(true);
        host.position.setPpqPosition(0.0);
        juce::AudioBuffer<float> audio(2,512); juce::MidiBuffer events;
        std::set<int> active; int received=0;
        for(int b=0;b<300;++b) {
            host.position.setPpqPosition(static_cast<double>(b)*512.0/24000.0);
            p.processBlock(audio,events);
            for(const auto meta:events) {
                require(meta.samplePosition>=0 && meta.samplePosition<512, "MIDI event inside buffer");
                auto m=meta.getMessage();
                if(m.isNoteOn()) {active.insert(m.getNoteNumber());++received;}
                else if(m.isNoteOff()) active.erase(m.getNoteNumber());
            }
            require(audio.getMagnitude(0,512)==0.0f, "MIDI generator has silent audio output");
        }
        require(received>0, "Processor emits MIDI under host transport");
        host.position.setIsPlaying(false);p.processBlock(audio,events);
        for(const auto meta:events) if(meta.getMessage().isNoteOff()) active.erase(meta.getMessage().getNoteNumber());
        require(active.empty(), "Transport stop releases all generated notes");
        require(!p.isHostPlaying(), "UI transport follows host");

        p.setEnabled(false);host.position.setIsPlaying(true);p.processBlock(audio,events);
        for(const auto meta:events) require(!meta.getMessage().isNoteOn(), "Disabled generator emits no notes");
        p.setEnabled(true);p.processBlockBypassed(audio,events);
        for(const auto meta:events) require(!meta.getMessage().isNoteOn(), "Bypassed generator emits no notes");

        // Lock history is visually meaningful, but must not re-attack a held note.
        const auto heldPattern=p.getPattern();int heldIndex=-1;
        for(int i=0;i<heldPattern.count;++i) if(heldPattern.notes[static_cast<size_t>(i)].duration>0.08) {heldIndex=i;break;}
        require(heldIndex>=0,"Fixture contains a note long enough for bypass/undo checks");
        const auto held=heldPattern.notes[static_cast<size_t>(heldIndex)];
        host.position.setPpqPosition(held.start+0.001);p.processBlock(audio,events);
        bool chased=false;
        for(const auto meta:events) if(meta.getMessage().isNoteOn() && meta.getMessage().getNoteNumber()==held.pitch) chased=true;
        require(chased,"Starting inside a held note chases it");
        p.toggleLock(heldIndex);p.undo();
        host.position.setPpqPosition(held.start+0.001+512.0/24000.0);p.processBlock(audio,events);
        require(events.isEmpty(),"Undoing a note lock must not re-attack a held note");
        p.redo();
        host.position.setPpqPosition(held.start+0.001+1024.0/24000.0);p.processBlock(audio,events);
        require(events.isEmpty(),"Redoing a note lock must not re-attack a held note");
        p.processBlockBypassed(audio,events);bool released=false;
        for(const auto meta:events) if(meta.getMessage().isNoteOff() && meta.getMessage().getNoteNumber()==held.pitch) released=true;
        require(released,"Bypassing an active note must release it");

        if(argc>1) {
            std::unique_ptr<juce::AudioProcessorEditor> editor(p.createEditor());
            require(editor!=nullptr,"Editor exists");
            editor->setVisible(true);
            auto image=editor->createComponentSnapshot(editor->getLocalBounds(),true,1.0f);
            juce::File out(juce::String::fromUTF8(argv[1]));auto stream=out.createOutputStream();
            if(stream) { stream->setPosition(0);stream->truncate(); }
            require(stream && juce::PNGImageFormat().writeImageToStream(image,*stream),"Editor screenshot written");
        }
        file.deleteFile();
        std::cout << "PASS: processor state, undo/redo, fresh seeds, export parse, MIDI routing, stop/bypass, editor\n";
        return 0;
    } catch(const std::exception& e) { std::cerr << "FAIL: " << e.what() << '\n';return 1; }
}
