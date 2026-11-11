#include "PluginProcessor.h"
#include <juce_audio_utils/juce_audio_utils.h>
#include <algorithm>
#include <array>
#include <cmath>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <vector>

namespace {
int checks = 0;
void require(bool condition, const juce::String& message)
{
    ++checks;
    if (!condition) throw std::runtime_error(message.toStdString());
}

juce::Component* find(juce::Component& parent, const juce::String& id)
{
    if (parent.getComponentID() == id) return &parent;
    for (auto* child : parent.getChildren())
        if (auto* match = find(*child, id)) return match;
    return nullptr;
}

template <typename T> T& control(juce::Component& editor, const juce::String& id)
{
    auto* result = dynamic_cast<T*>(find(editor, id));
    require(result != nullptr, "Expected editor control: " + id);
    return *result;
}

bool visibleInside(const juce::Component& component, const juce::Component& root)
{
    for (auto* current = &component; current != nullptr; current = current->getParentComponent()) {
        if (!current->isVisible()) return false;
        if (current == &root) return true;
    }
    return false;
}

// These exercise the real application callbacks and synchronous JUCE control
// notifications. Native mouse/drag gestures remain a separate host check.
void activate(juce::Component& editor, const juce::String& id)
{
    auto& button = control<juce::Button>(editor, id);
    require(button.isEnabled(), "Action is enabled: " + id);
    require(static_cast<bool>(button.onClick), "Action has a callback: " + id);
    if (button.getClickingTogglesState()) {
        const bool target = button.getRadioGroupId() != 0 || !button.getToggleState();
        if (target != button.getToggleState()) {
            // This also sends the radio-off callbacks that a real button click
            // sends to its old selection. Omitting them can hide UI regressions.
            button.setToggleState(target, juce::sendNotificationSync);
            return;
        }
    }
    button.onClick();
}

bool sameNote(const undertow::Note& a, const undertow::Note& b)
{
    return a.start == b.start && a.duration == b.duration && a.pitch == b.pitch
        && a.velocity == b.velocity && a.locked == b.locked;
}

bool samePattern(const undertow::Pattern& a, const undertow::Pattern& b)
{
    if (a.count != b.count || a.lengthBeats != b.lengthBeats || a.seed != b.seed) return false;
    for (int i = 0; i < a.count; ++i) if (!sameNote(a.notes[i], b.notes[i])) return false;
    return true;
}

bool contains(const undertow::Pattern& pattern, const undertow::Note& note)
{
    for (int i = 0; i < pattern.count; ++i) if (sameNote(pattern.notes[i], note)) return true;
    return false;
}

const std::array<const char*, 7> detailIds{
    "swing", "gate", "octave", "root-only", "root-anchor", "octave-jumps", "more-controls"};
const std::array<const char*, 12> essentialIds{
    "style",
    "key", "scale", "length", "density", "movement", "generate", "variation",
    "midi-output", "midi-drag", "help", "more-controls"};

void checkDisclosure(juce::Component& editor, bool expanded)
{
    juce::KeyboardFocusTraverser traverser;
    const auto focusable = traverser.getAllComponents(&editor);
    for (auto* id : detailIds) {
        if (juce::String(id) == "more-controls") continue;
        auto& item = control<juce::Component>(editor, id);
        require(visibleInside(item, editor) == expanded, "Detail visibility: " + juce::String(id));
        if (!expanded) {
            for (auto* focused : focusable)
                require(focused != &item && !item.isParentOf(focused),
                        "Hidden detail is absent from keyboard traversal: " + juce::String(id));
        }
    }
    for (auto* id : essentialIds)
        require(visibleInside(control<juce::Component>(editor, id), editor),
                "Essential control remains visible: " + juce::String(id));
}

void checkLayout(juce::AudioProcessorEditor& editor)
{
    std::vector<juce::Component*> controls;
    std::vector<juce::Rectangle<float>> bounds;
    const std::array<const char*, 21> ids{
        "style",
        "key", "scale", "length", "density", "movement", "generate", "variation",
        "midi-output", "midi-drag", "help", "more-controls", "undo", "redo", "unlock-all",
        "swing", "gate", "octave", "root-only", "root-anchor", "octave-jumps"};
    for (auto* id : ids) {
        auto& item = control<juce::Component>(editor, id);
        if (!visibleInside(item, editor)) continue;
        const auto rect = editor.getLocalArea(&item, item.getLocalBounds().toFloat());
        require(rect.getWidth() > 0 && rect.getHeight() > 0, "Visible control has area: " + juce::String(id));
        require(editor.getLocalBounds().toFloat().expanded(.5f).contains(rect),
                "Visible control stays inside resized editor: " + juce::String(id));
        require(item.getTitle().isNotEmpty() || item.getName().isNotEmpty(),
                "Control has an accessible name: " + juce::String(id));
        controls.push_back(&item);
        bounds.push_back(rect);
    }
    for (std::size_t a = 0; a < bounds.size(); ++a)
        for (std::size_t b = a + 1; b < bounds.size(); ++b)
            require(!bounds[a].reduced(.5f).intersects(bounds[b].reduced(.5f)),
                    "Interactive controls do not overlap: " + controls[a]->getComponentID()
                    + " / " + controls[b]->getComponentID());
}

void loadScreenshotFixture(UndertowProcessor& processor)
{
    // Only the test fixture chooses a fixed seed. The product still generates
    // fresh patterns; state restoration is the public path for exact recall.
    undertow::Settings settings;
    settings.style = undertow::Style::rolling;
    const auto pattern = undertow::generate(settings, 0xa341316cu);
    processor.setSettings(settings);
    processor.generateNew();
    juce::MemoryBlock state;
    processor.getStateInformation(state);
    auto xml = juce::AudioProcessor::getXmlFromBinary(state.getData(), static_cast<int>(state.getSize()));
    require(xml != nullptr, "Screenshot fixture state parses");
    auto* notes = xml->getChildByName("Pattern");
    require(notes != nullptr, "Screenshot fixture has a pattern");
    notes->deleteAllChildElements();
    notes->setAttribute("seed", juce::String(static_cast<juce::int64>(pattern.seed)));
    notes->setAttribute("length", pattern.lengthBeats);
    for (int i = 0; i < pattern.count; ++i) {
        const auto& note = pattern.notes[i];
        auto* value = notes->createNewChildElement("Note");
        value->setAttribute("start", juce::String(note.start, 17));
        value->setAttribute("duration", juce::String(note.duration, 17));
        value->setAttribute("pitch", note.pitch);
        value->setAttribute("velocity", note.velocity);
        value->setAttribute("locked", false);
    }
    juce::AudioProcessor::copyXmlToBinary(*xml, state);
    processor.setStateInformation(state.getData(), static_cast<int>(state.getSize()));
    require(samePattern(pattern, processor.getPattern()), "Screenshot fixture restores real generated notes exactly");
}

void saveSnapshot(juce::AudioProcessorEditor& editor, const juce::File& directory, const char* name)
{
    const auto path = directory.getChildFile(name);
    auto stream = path.createOutputStream();
    const auto image = editor.createComponentSnapshot(editor.getLocalBounds(), true, 1.0f);
    require(stream != nullptr && stream->setPosition(0) && stream->truncate().wasOk(),
            "Previous screenshot content is replaced: " + path.getFileName());
    require(juce::PNGImageFormat().writeImageToStream(image, *stream),
            "Snapshot written: " + path.getFileName());
    stream->flush();
    require(stream->getStatus().wasOk(), "Snapshot write completes: " + path.getFileName());
    const auto saved = juce::ImageFileFormat::loadFrom(path);
    require(saved.isValid() && saved.getWidth() == editor.getWidth()
            && saved.getHeight() == editor.getHeight(),
            "Saved screenshot contains the current editor dimensions: " + path.getFileName());
}
}

int main(int argc, char* argv[])
{
    juce::ScopedJuceInitialiser_GUI init;
    try {
        UndertowProcessor processor;
        std::unique_ptr<juce::AudioProcessorEditor> editor(processor.createEditor());
        require(editor != nullptr, "Native editor exists");
        editor->setVisible(true);
        require(editor->getWidth() <= 960 && editor->getHeight() <= 660,
                "Default editor is smaller than the old 1040x740 surface");
        checkDisclosure(*editor, false);
        checkLayout(*editor);
        require(control<juce::Button>(*editor, "variation").isEnabled(),
                "Changing a few notes is available for the current committed phrase");
        const auto original = processor.getPattern();
        const auto originalSize = editor->getBounds();
        activate(*editor, "more-controls");
        checkDisclosure(*editor, true);
        checkLayout(*editor);
        require(editor->getWidth() <= originalSize.getWidth()
                && editor->getHeight() > originalSize.getHeight(),
                "Disclosure adds height and may shrink width to fit the available display");
        require(samePattern(original, processor.getPattern()), "Opening details does not generate notes");

        control<juce::Slider>(*editor, "swing").setValue(.42, juce::sendNotificationSync);
        control<juce::Slider>(*editor, "gate").setValue(.27, juce::sendNotificationSync);
        control<juce::ComboBox>(*editor, "octave").setSelectedId(3, juce::sendNotificationSync);
        auto& only = control<juce::ToggleButton>(*editor, "root-only");
        only.setToggleState(true, juce::sendNotificationSync);
        require(processor.hasPendingSettings(), "Detail edits are pending until generation");
        require(!control<juce::Button>(*editor, "variation").isEnabled(),
                "Changing a few notes is disabled while setup changes are pending");
        require(samePattern(original, processor.getPattern()), "Detail edits preserve the displayed phrase");
        activate(*editor, "more-controls");
        checkDisclosure(*editor, false);
        require(editor->getWidth() <= originalSize.getWidth()
                && editor->getHeight() <= originalSize.getHeight(),
                "Collapsing details restores a compact editor size");
        activate(*editor, "more-controls");
        require(std::abs(control<juce::Slider>(*editor, "swing").getValue() - .42) < .00001,
                "Disclosure retains Swing");
        require(std::abs(control<juce::Slider>(*editor, "gate").getValue() - .27) < .00001,
                "Disclosure retains note length");
        require(control<juce::ComboBox>(*editor, "octave").getSelectedId() == 3
                && only.getToggleState(), "Disclosure retains pitch controls");

        auto& style = control<juce::ComboBox>(*editor, "style");
        for (int i = 0; i < 6; ++i) {
            style.setSelectedId(i + 1, juce::sendNotificationSync);
            require(static_cast<int>(processor.getSettings().style) == i,
                    "Style chooser selects its corresponding bassline type");
        }
        style.setSelectedId(5, juce::sendNotificationSync);
        require(processor.getSettings().style == undertow::Style::disco, "Style chooser selects Disco");
        control<juce::ComboBox>(*editor, "key").setSelectedId(8, juce::sendNotificationSync);
        control<juce::ComboBox>(*editor, "length").setSelectedId(4, juce::sendNotificationSync);
        require(samePattern(original, processor.getPattern()), "Setup choices do not silently replace notes");
        activate(*editor, "generate");
        const auto generated = processor.getPattern();
        require(!processor.hasPendingSettings(), "Generate commits chosen controls");
        require(control<juce::Button>(*editor, "variation").isEnabled(),
                "Changing a few notes is enabled after Generate commits the setup");
        require(generated.lengthBeats == 16, "Generate applies chosen four-bar length");
        require(generated.seed != original.seed, "Generate runs with a fresh seed");
        for (int i = 0; i < generated.count; ++i)
            require(generated.notes[i].pitch % 12 == 7 && generated.notes[i].pitch >= 55,
                    "Generate applies root-only G and selected register");

        auto& roll = control<juce::Component>(*editor, "piano-roll");
        require(roll.keyPressed(juce::KeyPress(juce::KeyPress::leftKey)),
                "Piano roll accepts initial left-arrow selection");
        require(roll.keyPressed(juce::KeyPress(juce::KeyPress::spaceKey)),
                "Piano roll accepts keyboard note locking");
        require(processor.getPattern().notes[generated.count - 1].locked,
                "Initial left-arrow selection reaches the last note");
        require(roll.keyPressed(juce::KeyPress(juce::KeyPress::rightKey)),
                "Piano roll wraps right-arrow selection to the first note");
        require(roll.keyPressed(juce::KeyPress(juce::KeyPress::spaceKey)),
                "Piano roll accepts keyboard note locking");
        const auto locked = processor.getPattern();
        require(locked.notes[0].locked, "Keyboard note locking reaches the displayed phrase");
        activate(*editor, "variation");
        const auto varied = processor.getPattern();
        require(contains(varied, locked.notes[0]), "Vary retains exact locked notes");
        require(contains(varied, locked.notes[locked.count - 1]),
                "Changing a few notes retains the keyboard-locked last note exactly");
        require(varied.count == locked.count, "Vary does not add an extra rhythm layer");
        activate(*editor, "undo");
        require(samePattern(locked, processor.getPattern()), "Editor Undo restores the exact phrase");
        activate(*editor, "redo");
        require(samePattern(varied, processor.getPattern()), "Editor Redo restores the exact variation");
        auto& unlock = control<juce::Button>(*editor, "unlock-all");
        require(visibleInside(unlock, *editor), "Unlock all is available when notes are locked");
        activate(*editor, "unlock-all");
        for (int i = 0; i < processor.getPattern().count; ++i)
            require(!processor.getPattern().notes[i].locked, "Unlock all clears locks");
        require(!visibleInside(unlock, *editor), "Unlock all is hidden when there are no locks");

        auto& output = control<juce::ToggleButton>(*editor, "midi-output");
        output.setToggleState(false, juce::sendNotificationSync);
        require(!processor.isEnabled(), "Visible MIDI switch disables generator output");
        output.setToggleState(true, juce::sendNotificationSync);
        require(processor.isEnabled(), "Visible MIDI switch re-enables generator output");

        auto* constraints = editor->getConstrainer();
        require(constraints != nullptr, "Resizable editor has bounds constraints");
        for (auto size : {juce::Point<int>(constraints->getMinimumWidth(), constraints->getMinimumHeight()),
                          juce::Point<int>(constraints->getMaximumWidth(), constraints->getMaximumHeight())}) {
            editor->setSize(size.x, size.y);
            checkLayout(*editor);
            activate(*editor, "more-controls");
            checkLayout(*editor);
            activate(*editor, "more-controls");
        }
        editor.reset();

        if (argc > 1) {
            const juce::File directory(juce::String::fromUTF8(argv[1]));
            require(directory.createDirectory().wasOk(), "Screenshot folder is writable");
            UndertowProcessor fixture;
            loadScreenshotFixture(fixture);
            std::unique_ptr<juce::AudioProcessorEditor> view(fixture.createEditor());
            view->setVisible(true);
            saveSnapshot(*view, directory, "default.png");
            activate(*view, "more-controls");
            saveSnapshot(*view, directory, "expanded.png");
            activate(*view, "more-controls");
            auto* limits = view->getConstrainer();
            view->setSize(limits->getMinimumWidth(), limits->getMinimumHeight());
            saveSnapshot(*view, directory, "minimum.png");
            view->setSize(limits->getMaximumWidth(), limits->getMaximumHeight());
            saveSnapshot(*view, directory, "large.png");
            std::cout << "Native editor screenshots: " << directory.getFullPathName() << '\n';
        }
        std::cout << "PASS: " << checks
                  << " editor checks (disclosure, settings, generation, history, locks, output, layout).\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "FAIL: " << error.what() << '\n';
        return 1;
    }
}
