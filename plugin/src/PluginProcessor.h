#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "PatternEngine.h"
#include <atomic>
#include <deque>
#include <mutex>

class UndertowProcessor final : public juce::AudioProcessor {
public:
    UndertowProcessor();
    ~UndertowProcessor() override = default;
    void prepareToPlay(double, int) override;
    void releaseResources() override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    void processBlockBypassed(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    bool isBusesLayoutSupported(const BusesLayout&) const override;
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return JucePlugin_Name; }
    // Live requires an input event bus for VST3 instruments. Incoming events are
    // discarded; only the generator's notes may reach the output.
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return true; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return "Default"; }
    void changeProgramName(int, const juce::String&) override {}
    void getStateInformation(juce::MemoryBlock&) override;
    void setStateInformation(const void*, int) override;

    // UI access is message-thread only; the audio thread takes a non-blocking snapshot.
    undertow::Settings getSettings() const;
    undertow::Pattern getPattern() const;
    bool hasPendingSettings() const;
    void setSettings(undertow::Settings); // Updates controls; Generate applies them.
    bool generateNew(bool variation = false);
    int getRegisterShift() const;
    bool canSetRegisterShift(int octaves) const;
    bool setRegisterShift(int octaves);
    void toggleLock(int noteIndex);
    void clearLocks();
    void undo();
    void redo();
    bool canUndo() const;
    bool canRedo() const;
    void setEnabled(bool);
    bool isEnabled() const noexcept { return enabled.load(); }
    bool isHostPlaying() const noexcept { return hostPlaying.load(); }
    bool hasHostPosition() const noexcept { return hostPosition.load(); }
    double getBpm() const noexcept { return hostBpm.load(); }
    double getPlayheadBeat() const noexcept { return playheadBeat.load(); }
    std::uint64_t getRevision() const noexcept { return revision.load(); }
    juce::File createMidiDragFile();
    bool exportMidiTo(const juce::File&);
    void panic() noexcept { panicRequested.store(true); }

private:
    struct Snapshot { undertow::Settings settings, committedSettings; undertow::Pattern pattern; int registerShift; };
    void remember();
    void commitPattern(const undertow::Pattern&, int shift);
    void process(juce::AudioBuffer<float>&, juce::MidiBuffer&, bool bypassed);
    mutable std::mutex stateMutex;
    undertow::Settings settings;
    undertow::Settings committedSettings;
    undertow::Pattern pattern;
    int registerShift = 0; // Canonical notes stay in the generation register.
    std::deque<Snapshot> undoStack, redoStack;
    undertow::Pattern audioPattern;
    undertow::Scheduler scheduler;
    std::uint64_t audioRevision = 0;
    std::atomic<std::uint64_t> revision{1};
    std::atomic<std::uint64_t> playbackRevision{1};
    std::atomic<bool> enabled{true}, hostPlaying{false}, hostPosition{false}, panicRequested{false};
    std::atomic<double> hostBpm{120.0}, playheadBeat{-1.0};
    juce::Random random;
    double rate = 44100.0;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(UndertowProcessor)
};
