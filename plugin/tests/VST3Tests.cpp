// A small native VST3 host: test the shipped wrapper, not just processBlock.
#include <CoreFoundation/CoreFoundation.h>
#include "pluginterfaces/base/ipluginbase.h"
#include "pluginterfaces/base/ibstream.h"
#include "pluginterfaces/vst/ivstcomponent.h"
#include "pluginterfaces/vst/ivstaudioprocessor.h"
#include "pluginterfaces/vst/ivsthostapplication.h"
#include "pluginterfaces/vst/ivstevents.h"
#include <algorithm>
#include <array>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

using namespace Steinberg;
using namespace Steinberg::Vst;
namespace {
void require(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}
template<class Interface> class HostObject : public Interface {
public:
    tresult PLUGIN_API queryInterface(const TUID id, void** object) override {
        if (std::memcmp(id, Interface::iid.toTUID(), sizeof(TUID)) == 0
            || std::memcmp(id, FUnknown::iid.toTUID(), sizeof(TUID)) == 0) {
            *object = static_cast<Interface*>(this); addRef(); return kResultOk;
        }
        *object = nullptr; return kNoInterface;
    }
    uint32 PLUGIN_API addRef() override { return ++references; }
    uint32 PLUGIN_API release() override { return --references; }
private:
    // The fixture owns these objects and outlives all plug-in references.
    uint32 references = 1;
};
class Host final : public HostObject<IHostApplication> {
public:
    tresult PLUGIN_API getName(String128 name) override {
        const char16_t label[] = u"Submotif Regression Host";
        std::copy(std::begin(label), std::end(label), name); return kResultOk;
    }
    tresult PLUGIN_API createInstance(TUID, TUID, void** object) override {
        *object = nullptr; return kNoInterface;
    }
};
class Events final : public HostObject<IEventList> {
public:
    std::vector<Event> events;
    int32 PLUGIN_API getEventCount() override { return static_cast<int32>(events.size()); }
    tresult PLUGIN_API getEvent(int32 index, Event& event) override {
        if (index < 0 || index >= getEventCount()) return kInvalidArgument;
        event = events[static_cast<size_t>(index)]; return kResultOk;
    }
    tresult PLUGIN_API addEvent(Event& event) override { events.push_back(event); return kResultOk; }
    void feedExternal(bool zeroSamples) {
        events.clear();
        Event note{}; note.type = Event::kNoteOnEvent; note.busIndex = 0;
        note.noteOn.channel = 6; note.noteOn.pitch = 105; note.noteOn.velocity = 1.0f;
        note.noteOn.noteId = -1; events.push_back(note);
        note.type = Event::kNoteOffEvent; note.sampleOffset = zeroSamples ? 0 : 31;
        note.noteOff.channel = 6; note.noteOff.pitch = 105; note.noteOff.noteId = -1;
        events.push_back(note);
        Event pressure{}; pressure.type = Event::kPolyPressureEvent;
        pressure.polyPressure.channel = 6; pressure.polyPressure.pitch = 105;
        pressure.polyPressure.pressure = 1.0f; pressure.polyPressure.noteId = -1;
        events.push_back(pressure);
    }
};
class Stream final : public HostObject<IBStream> {
public:
    std::vector<char> bytes;
    int64 position = 0;
    tresult PLUGIN_API read(void* buffer, int32 requested, int32* actual) override {
        if (requested < 0) return kInvalidArgument;
        const auto count = static_cast<int32>(std::min<int64>(requested, std::max<int64>(0, static_cast<int64>(bytes.size()) - position)));
        if (count > 0) std::memcpy(buffer, bytes.data() + position, static_cast<size_t>(count));
        position += count; if (actual) *actual = count;
        return count > 0 ? kResultOk : kResultFalse;
    }
    tresult PLUGIN_API write(void* buffer, int32 count, int32* actual) override {
        if (count < 0) return kInvalidArgument;
        bytes.resize(static_cast<size_t>(std::max<int64>(bytes.size(), position + count)));
        if (count > 0) std::memcpy(bytes.data() + position, buffer, static_cast<size_t>(count));
        position += count; if (actual) *actual = count; return kResultOk;
    }
    tresult PLUGIN_API seek(int64 offset, int32 mode, int64* result) override {
        const auto next = (mode == kIBSeekSet ? 0 : mode == kIBSeekCur ? position : static_cast<int64>(bytes.size())) + offset;
        if (next < 0) return kInvalidArgument;
        position = next; if (result) *result = position; return kResultOk;
    }
    tresult PLUGIN_API tell(int64* result) override { if (!result) return kInvalidArgument; *result = position; return kResultOk; }
};
class Plugin final {
public:
    explicit Plugin(const char* path) {
        auto url = CFURLCreateFromFileSystemRepresentation(nullptr, reinterpret_cast<const UInt8*>(path), std::strlen(path), true);
        require(url != nullptr, "Bundle URL created");
        bundle = CFBundleCreate(nullptr, url); CFRelease(url);
        require(bundle && CFBundleLoadExecutable(bundle), "Built VST3 bundle loads");
        auto entry = reinterpret_cast<bool(*)(CFBundleRef)>(symbol(CFSTR("bundleEntry")));
        exit = reinterpret_cast<bool(*)()>(symbol(CFSTR("bundleExit")));
        auto getFactory = reinterpret_cast<IPluginFactory*(*)()>(symbol(CFSTR("GetPluginFactory")));
        require(entry && exit && getFactory && entry(bundle), "VST3 entry points initialize");
        entered = true; factory = getFactory(); require(factory != nullptr, "VST3 factory exists");
        for (int32 index = 0; index < factory->countClasses(); ++index) {
            PClassInfo info{};
            if (factory->getClassInfo(index, &info) == kResultOk && std::strcmp(info.category, kVstAudioEffectClass) == 0) {
                require(factory->createInstance(info.cid, IComponent::iid.toTUID(), reinterpret_cast<void**>(&component)) == kResultOk,
                        "Factory creates instrument component"); break;
            }
        }
        require(component != nullptr, "Instrument component found");
        require(component->initialize(&host) == kResultOk, "Component initializes"); initialized = true;
        require(component->queryInterface(IAudioProcessor::iid.toTUID(), reinterpret_cast<void**>(&processor)) == kResultOk,
                "Component exposes audio processor");
    }
    ~Plugin() {
        if (processor) { processor->setProcessing(false); processor->release(); }
        if (component) { component->setActive(false); if (initialized) component->terminate(); component->release(); }
        if (factory) factory->release();
        if (entered) exit();
        if (bundle) { CFBundleUnloadExecutable(bundle); CFRelease(bundle); }
    }
    void start() {
        require(component->getBusCount(kEvent, kInput) == 1, "Live requires one instrument MIDI input bus");
        require(component->getBusCount(kEvent, kOutput) == 1, "Generator exposes one MIDI output bus");
        BusInfo input{};
        require(component->getBusInfo(kEvent, kInput, 0, input) == kResultOk
                && input.busType == kMain && input.channelCount == 16, "Input event bus is valid main 16-channel bus");
        require(component->activateBus(kEvent, kInput, 0, true) == kResultOk, "MIDI input bus activates");
        require(component->activateBus(kEvent, kOutput, 0, true) == kResultOk, "MIDI output bus activates");
        require(component->activateBus(kAudio, kOutput, 0, true) == kResultOk, "Silent audio output activates");
        ProcessSetup setup{kRealtime, kSample32, 512, 48000.0};
        require(processor->setupProcessing(setup) == kResultOk, "Processing setup succeeds");
        require(component->setActive(true) == kResultOk, "Instrument activates");
        require(processor->setProcessing(true) == kResultOk, "Instrument starts processing");
    }
    void disableOutput() {
        Stream state;
        require(component->getState(&state) == kResultOk, "Read native component state");
        const std::string marker = "enabled=\"1\"";
        const auto found = std::search(state.bytes.begin(), state.bytes.end(), marker.begin(), marker.end());
        require(found != state.bytes.end(), "State contains enabled generator");
        *(found + static_cast<std::ptrdiff_t>(marker.size() - 2)) = '0';
        state.position = 0;
        require(component->setState(&state) == kResultOk, "Disable Live MIDI through native component state");
        Stream readback; require(component->getState(&readback) == kResultOk, "Read disabled state");
        const std::string disabled = "enabled=\"0\"";
        require(std::search(readback.bytes.begin(), readback.bytes.end(), disabled.begin(), disabled.end()) != readback.bytes.end(),
                "Live MIDI output is disabled");
    }
    int process(int block, bool flush, bool allowNotes) {
        Events input, output; input.feedExternal(flush);
        std::array<float, 512> left{}, right{}; float* channels[]{left.data(), right.data()};
        AudioBusBuffers audio{}; audio.numChannels = 2; audio.channelBuffers32 = channels;
        ProcessContext context{}; context.state = ProcessContext::kPlaying | ProcessContext::kTempoValid | ProcessContext::kProjectTimeMusicValid;
        context.sampleRate = 48000.0; context.tempo = 120.0;
        context.projectTimeSamples = static_cast<int64>(block) * 512;
        context.projectTimeMusic = static_cast<double>(block) * 512.0 / 24000.0;
        ProcessData data{}; data.processMode = kRealtime; data.symbolicSampleSize = kSample32;
        data.numSamples = flush ? 0 : 512; data.numOutputs = flush ? 0 : 1;
        data.outputs = flush ? nullptr : &audio; data.inputEvents = &input; data.outputEvents = &output;
        data.processContext = &context;
        require(processor->process(data) == kResultOk, "Native VST3 processing succeeds");
        if (flush) require(output.events.empty(), "Zero-audio parameter flush never echoes incoming MIDI");
        int noteOns = 0;
        for (const auto& event : output.events) {
            require(event.type == Event::kNoteOnEvent || event.type == Event::kNoteOffEvent,
                    "External expression/controller messages cannot reach output");
            const auto channel = event.type == Event::kNoteOnEvent ? event.noteOn.channel : event.noteOff.channel;
            const auto pitch = event.type == Event::kNoteOnEvent ? event.noteOn.pitch : event.noteOff.pitch;
            require(channel == 0 && pitch != 105, "Native wrapper outputs generated notes only, never external MIDI");
            if (event.type == Event::kNoteOnEvent) { require(allowNotes, "Disabled output emits no note-ons"); ++noteOns; }
        }
        return noteOns;
    }
private:
    void* symbol(CFStringRef name) { return reinterpret_cast<void*>(CFBundleGetFunctionPointerForName(bundle, name)); }
    Host host;
    CFBundleRef bundle = nullptr;
    bool (*exit)() = nullptr;
    IPluginFactory* factory = nullptr;
    IComponent* component = nullptr;
    IAudioProcessor* processor = nullptr;
    bool entered = false, initialized = false;
};
}
int main(int argc, char** argv) {
    try {
        require(argc == 2, "Pass the built .vst3 bundle path");
        Plugin plugin(argv[1]); plugin.start();
        plugin.process(0, true, true);
        int generated = 0;
        for (int block = 0; block < 240; ++block) generated += plugin.process(block, false, true);
        require(generated > 0, "Native wrapper still emits generated bass notes under transport");
        plugin.process(240, true, true);
        plugin.disableOutput();
        for (int block = 240; block < 244; ++block) plugin.process(block, false, false);
        plugin.process(244, true, false);
        std::cout << "PASS: native VST3 input/output buses, generated notes, external MIDI isolation, disabled output, zero-audio flush\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "FAIL: " << error.what() << '\n'; return 1;
    }
}
