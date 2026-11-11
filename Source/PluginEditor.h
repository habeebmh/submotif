#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <memory>

class UndertowProcessor;

class UndertowEditor final : public juce::AudioProcessorEditor {
public:
    explicit UndertowEditor(UndertowProcessor&);
    ~UndertowEditor() override;
    void resized() override;
    void paint(juce::Graphics&) override;

private:
    class Surface;
    std::unique_ptr<Surface> surface;
    void setDetailsExpanded(bool);
    bool detailsExpanded = false;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(UndertowEditor)
};
