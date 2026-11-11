#include "PluginEditor.h"
#include "PluginProcessor.h"
#include <algorithm>
#include <cmath>

namespace {
constexpr int designWidth = 920, designHeight = 560, expandedHeight = 700;
const juce::Colour background{0xff191c1e}, panel{0xff24282a}, inset{0xff141719};
const juce::Colour border{0xff383e40}, ink{0xfff0f2ed}, muted{0xffa7afad};
const juce::Colour accent{0xffc5ed76}, dim{0xff77817f};

juce::Font font(float height, bool bold = false)
{
    return juce::Font(juce::FontOptions("Avenir Next", bold ? "Demi Bold" : "Regular", height));
}

void labelText(juce::Graphics& g, juce::String s, juce::Rectangle<int> r, float size,
          juce::Colour colour = ink, bool bold = false,
          juce::Justification align = juce::Justification::centredLeft)
{
    g.setColour(colour);
    g.setFont(font(size, bold));
    g.drawFittedText(s, r, align, 1, 0.9f);
}

bool sameSettings(const undertow::Settings& a, const undertow::Settings& b)
{
    return a.root == b.root && a.scale == b.scale && a.style == b.style && a.bars == b.bars
        && a.octave == b.octave && a.density == b.density && a.swing == b.swing
        && a.gate == b.gate && a.movement == b.movement && a.rootOnly == b.rootOnly
        && a.anchorRoot == b.anchorRoot && a.octaveJumps == b.octaveJumps;
}

class Theme final : public juce::LookAndFeel_V4 {
public:
    Theme()
    {
        setColour(juce::ComboBox::backgroundColourId, panel);
        setColour(juce::ComboBox::textColourId, ink);
        setColour(juce::ComboBox::outlineColourId, border);
        setColour(juce::ComboBox::arrowColourId, muted);
        setColour(juce::PopupMenu::backgroundColourId, panel);
        setColour(juce::PopupMenu::textColourId, ink);
        setColour(juce::PopupMenu::highlightedBackgroundColourId, accent);
        setColour(juce::PopupMenu::highlightedTextColourId, background);
        setColour(juce::TooltipWindow::backgroundColourId, juce::Colour(0xff30392c));
        setColour(juce::TooltipWindow::textColourId, ink);
        setColour(juce::TooltipWindow::outlineColourId, border);
        setColour(juce::AlertWindow::backgroundColourId, panel);
        setColour(juce::AlertWindow::textColourId, ink);
        setColour(juce::AlertWindow::outlineColourId, border);
        setColour(juce::TextButton::buttonColourId, panel);
        setColour(juce::TextButton::textColourOffId, ink);
        setColour(juce::TextButton::textColourOnId, background);
        setColour(juce::Label::textColourId, ink);
    }

    juce::Font getComboBoxFont(juce::ComboBox&) override { return font(14.0f); }
    juce::Font getPopupMenuFont() override { return font(14.0f); }
    juce::Font getTextButtonFont(juce::TextButton&, int) override { return font(13.0f, true); }

    void drawComboBox(juce::Graphics& g, int width, int height, bool, int, int, int, int,
                      juce::ComboBox& box) override
    {
        auto r = juce::Rectangle<float>(0.5f, 0.5f, (float)width - 1, (float)height - 1);
        g.setColour(panel);
        g.fillRoundedRectangle(r, 7.0f);
        g.setColour(box.hasKeyboardFocus(true) ? accent : border);
        g.drawRoundedRectangle(r, 7.0f, 1.0f);
        juce::Path p;
        const auto x = (float)width - 18.0f, y = (float)height * 0.5f;
        p.startNewSubPath(x - 3, y - 1.5f); p.lineTo(x, y + 1.5f); p.lineTo(x + 3, y - 1.5f);
        g.setColour(muted);
        g.strokePath(p, juce::PathStrokeType(1.4f));
    }

    void positionComboBoxText(juce::ComboBox& box, juce::Label& label) override
    {
        label.setBounds(12, 0, box.getWidth() - 36, box.getHeight());
        label.setFont(getComboBoxFont(box));
    }

    void drawButtonBackground(juce::Graphics& g, juce::Button& button, const juce::Colour&,
                              bool over, bool down) override
    {
        const bool primary = button.getProperties().getWithDefault("primary", false);
        const bool quiet = button.getProperties().getWithDefault("quiet", false);
        const bool choice = button.getProperties().getWithDefault("styleChoice", false);
        const bool selected = choice && button.getToggleState();
        auto r = button.getLocalBounds().toFloat().reduced(0.5f);
        if (down) r.translate(0, 1);
        auto fill = primary ? accent : (selected ? juce::Colour(0xff313d2d) : panel);
        if (over) fill = fill.brighter(primary ? .06f : .08f);
        if (down) fill = fill.darker(.08f);
        if (!button.isEnabled()) fill = fill.withAlpha(.3f);
        if (!quiet || over || down) {
            g.setColour(fill); g.fillRoundedRectangle(r, 7);
        }
        if (!quiet || button.hasKeyboardFocus(true)) {
            g.setColour(button.hasKeyboardFocus(true) ? ink : (selected ? accent.withAlpha(.65f) : (primary ? accent : border)));
            g.drawRoundedRectangle(r, 7, button.hasKeyboardFocus(true) ? 1.5f : 1.0f);
        }
    }

    void drawButtonText(juce::Graphics& g, juce::TextButton& button, bool, bool down) override
    {
        const bool primary = button.getProperties().getWithDefault("primary", false);
        const bool choice = button.getProperties().getWithDefault("styleChoice", false);
        const bool quiet = button.getProperties().getWithDefault("quiet", false);
        auto colour = primary ? background : (choice && button.getToggleState() ? accent : (quiet ? muted : ink));
        if (!button.isEnabled()) colour = dim.withAlpha(.65f);
        auto textArea = button.getLocalBounds().reduced(8, 0).translated(0, down ? 1 : 0);
        if ((bool)button.getProperties().getWithDefault("disclosure", false)) {
            textArea.removeFromRight(18);
            const float x = (float)button.getWidth() - 15, y = (float)button.getHeight() * .5f;
            const float direction = button.getToggleState() ? -1.0f : 1.0f;
            juce::Path chevron;
            chevron.startNewSubPath(x - 3, y - 1.5f * direction);
            chevron.lineTo(x, y + 1.5f * direction); chevron.lineTo(x + 3, y - 1.5f * direction);
            g.setColour(muted); g.strokePath(chevron, juce::PathStrokeType(1.2f));
        }
        const bool inlineAction = button.getProperties().getWithDefault("inlineAction", false);
        labelText(g, button.getButtonText(), textArea, primary ? 15.0f : 14.0f,
                  colour, primary || choice, inlineAction ? juce::Justification::centredLeft : juce::Justification::centred);
    }

    void drawToggleButton(juce::Graphics& g, juce::ToggleButton& button, bool over, bool) override
    {
        const bool on = button.getToggleState();
        const auto r = button.getLocalBounds().toFloat();
        const auto y = r.getCentreY() - 8;
        g.setColour(on ? accent : (over ? border.brighter(.15f) : border));
        g.fillRoundedRectangle(0, y, 28, 16, 8);
        g.setColour(on ? background : muted);
        g.fillEllipse(on ? 15.0f : 3.0f, y + 3, 10, 10);
        labelText(g, button.getButtonText(), {38, 0, button.getWidth() - 38, button.getHeight()}, 13.5f, ink);
        if (button.hasKeyboardFocus(true)) {
            g.setColour(ink); g.drawRoundedRectangle(r.reduced(.5f), 5, 1.5f);
        }
    }

    void drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
                          float position, float, float, const juce::Slider::SliderStyle,
                          juce::Slider& slider) override
    {
        const float centreY = (float)y + (float)height * .5f;
        g.setColour(border);
        g.drawLine((float)x, centreY, (float)(x + width), centreY, 3);
        g.setColour(accent);
        g.drawLine((float)x, centreY, position, centreY, 3);
        g.setColour(slider.isMouseOverOrDragging() ? ink : accent);
        g.fillEllipse(position - 6, centreY - 6, 12, 12);
        if (slider.hasKeyboardFocus(true)) {
            g.setColour(ink); g.drawEllipse(position - 10, centreY - 10, 20, 20, 1.5f);
        }
    }

};

class PianoRoll final : public juce::Component, public juce::SettableTooltipClient {
public:
    PianoRoll()
    {
        setWantsKeyboardFocus(true);
        setTitle("Bassline piano roll");
        setDescription("Click notes to lock them. Use left and right arrows to select a note, then space to lock it.");
        setTooltip("Click a note to keep its pitch, timing and velocity when generating. Arrow keys select; space locks.");
    }

    void setPattern(const undertow::Pattern& next)
    {
        pattern = next;
        lowPitch = 36; highPitch = 48;
        if (pattern.count > 0) {
            lowPitch = highPitch = pattern.notes[0].pitch;
            for (int i = 0; i < pattern.count; ++i) {
                lowPitch = juce::jmin(lowPitch, pattern.notes[(size_t)i].pitch);
                highPitch = juce::jmax(highPitch, pattern.notes[(size_t)i].pitch);
            }
        }
        lowPitch = juce::jmax(0, lowPitch - 2);
        highPitch = juce::jmin(127, juce::jmax(highPitch + 2, lowPitch + 11));
        selected = juce::jlimit(-1, pattern.count - 1, selected);
        repaint();
    }

    void setPlayhead(double beat) { if (beat != playhead) { playhead = beat; repaint(); } }
    std::function<void(int)> onLock;

    void paint(juce::Graphics& g) override
    {
        const auto area = grid();
        const auto beatWidth = area.getWidth() / (float)pattern.lengthBeats;
        const auto rowHeight = area.getHeight() / (float)(highPitch - lowPitch + 1);
        g.setColour(inset);
        g.fillAll();

        for (int pitch = lowPitch; pitch <= highPitch; ++pitch) {
            const auto pc = pitch % 12;
            const bool black = pc == 1 || pc == 3 || pc == 6 || pc == 8 || pc == 10;
            const auto y = area.getY() + (float)(highPitch - pitch) * rowHeight;
            g.setColour(black ? juce::Colour(0xff161a1c) : juce::Colour(0xff1a1f21));
            g.fillRect(area.getX(), y, area.getWidth(), rowHeight);
            g.setColour(black ? juce::Colour(0xff1b2022) : juce::Colour(0xff242a2c));
            g.fillRect(0.0f, y, area.getX() - 1.0f, rowHeight);
            const bool used = std::any_of(pattern.notes.begin(), pattern.notes.begin() + pattern.count, [pitch](const auto& n) { return n.pitch == pitch; });
            if (rowHeight >= 13 || pc == 0 || used)
                labelText(g, undertow::noteName(pitch), {5, juce::roundToInt(y), 35, juce::roundToInt(rowHeight)},
                     juce::jlimit(10.0f, 12.0f, rowHeight - 1.0f), pc == 0 || used ? ink : muted, false,
                     juce::Justification::centredRight);
            g.setColour(background.withAlpha(0.7f));
            g.drawHorizontalLine(juce::roundToInt(y + rowHeight), area.getX(), area.getRight());
        }

        const int steps = juce::roundToInt(pattern.lengthBeats * 4.0);
        for (int step = 0; step <= steps; ++step) {
            const auto x = area.getX() + (float)step * beatWidth * 0.25f;
            const bool barLine = step % 16 == 0;
            const bool beatLine = step % 4 == 0;
            g.setColour(barLine ? border.brighter(0.08f) : (beatLine ? border.withAlpha(0.45f) : border.withAlpha(0.14f)));
            g.drawVerticalLine(juce::roundToInt(x), area.getY(), (float)getHeight() - 8.0f);
            if (barLine && step < steps) {
                labelText(g, juce::String(step / 16 + 1), {juce::roundToInt(x + 6), 0, 40, 21},
                     12.0f, ink, true);
            } else if (beatLine && pattern.lengthBeats <= 16.0) {
                labelText(g, juce::String((step % 16) / 4 + 1), {juce::roundToInt(x + 5), 0, 25, 21},
                     10.0f, dim);
            }
        }
        for (int i = 0; i < pattern.count; ++i) {
            const auto& n = pattern.notes[(size_t)i];
            const auto r = noteBounds(i);
            const auto c = n.locked ? juce::Colour(0xfff0c58b) : accent.withMultipliedBrightness(0.83f + (float)n.velocity / 750.0f);
            g.setColour(c);
            g.fillRoundedRectangle(r, 2.0f);
            if (i == selected && hasKeyboardFocus(true)) {
                g.setColour(ink);
                g.drawRoundedRectangle(r.expanded(2), 3, 1);
            }
            if (r.getWidth() > 32.0f && r.getHeight() >= 10.0f)
                labelText(g, undertow::noteName(n.pitch), r.reduced(4, 0).toNearestInt(), 9.0f, background, true);
            if (n.locked && r.getWidth() > 12.0f) {
                g.setColour(background);
                const auto lockX = r.getRight() - 8.0f;
                g.fillRoundedRectangle(lockX, r.getCentreY() - 1, 4, 4, 0.5f);
                g.drawRoundedRectangle(lockX + 0.7f, r.getCentreY() - 3.0f, 2.6f, 3.5f, 1.0f, 0.8f);
            }
        }
        if (playhead >= 0 && playhead < pattern.lengthBeats) {
            const auto x = area.getX() + (float)playhead * beatWidth;
            g.setColour(ink.withAlpha(0.85f));
            g.drawVerticalLine(juce::roundToInt(x), area.getY(), (float)getHeight() - 8.0f);
            juce::Path p;
            p.addTriangle(x - 4, area.getY() - 5, x + 4, area.getY() - 5, x, area.getY());
            g.setColour(accent); g.fillPath(p);
        }
        if (hasKeyboardFocus(true)) {
            g.setColour(accent.withAlpha(0.6f));
            g.drawRect(getLocalBounds(), 1);
        }
    }

    void mouseDown(const juce::MouseEvent& e) override
    {
        grabKeyboardFocus();
        selected = findNote(e.position);
        if (selected >= 0 && onLock) onLock(selected);
        repaint();
    }
    void mouseMove(const juce::MouseEvent& e) override
    {
        const int index = findNote(e.position);
        setMouseCursor(index >= 0 ? juce::MouseCursor::PointingHandCursor : juce::MouseCursor::NormalCursor);
        if (index >= 0) {
            const auto& n = pattern.notes[(size_t)index];
            setTooltip(juce::String(undertow::noteName(n.pitch)) + " / MIDI " + juce::String(n.pitch)
                       + " / velocity " + juce::String(n.velocity) + (n.locked ? " — click to unlock" : " — click to lock"));
        } else setTooltip("Click a note to lock it. Arrow keys select; space locks. Amber notes stay unchanged when generating.");
    }
    bool keyPressed(const juce::KeyPress& key) override
    {
        if (pattern.count == 0) return false;
        if (key == juce::KeyPress::leftKey || key == juce::KeyPress::rightKey) {
            const int delta = key == juce::KeyPress::leftKey ? -1 : 1;
            selected = selected < 0 ? (delta > 0 ? 0 : pattern.count - 1) : (selected + delta + pattern.count) % pattern.count;
            repaint(); return true;
        }
        if (key == juce::KeyPress::spaceKey && selected >= 0) {
            if (onLock) onLock(selected);
            return true;
        }
        return false;
    }
    void focusGained(FocusChangeType) override { repaint(); }
    void focusLost(FocusChangeType) override { repaint(); }

private:
    juce::Rectangle<float> grid() const { return {47.0f, 25.0f, (float)getWidth() - 48, (float)getHeight() - 31}; }
    juce::Rectangle<float> noteBounds(int index) const
    {
        const auto r = grid();
        const auto& n = pattern.notes[(size_t)index];
        const auto row = r.getHeight() / (float)(highPitch - lowPitch + 1);
        return {r.getX() + (float)(n.start / pattern.lengthBeats) * r.getWidth() + 1.0f,
                r.getY() + (float)(highPitch - n.pitch) * row + 1.0f,
                juce::jmax(2.0f, (float)(n.duration / pattern.lengthBeats) * r.getWidth() - 2.0f),
                juce::jmax(2.0f, row - 2.0f)};
    }
    int findNote(juce::Point<float> p) const
    {
        for (int i = 0; i < pattern.count; ++i)
            if (noteBounds(i).expanded(0, 1).contains(p)) return i;
        return -1;
    }
    undertow::Pattern pattern;
    int lowPitch = 34, highPitch = 51, selected = -1;
    double playhead = -1;
};

class MidiDragButton final : public juce::Button {
public:
    MidiDragButton() : Button("Drag MIDI")
    {
        setMouseCursor(juce::MouseCursor::DraggingHandCursor);
        setTitle("Drag MIDI file");
        setDescription("Drag the current bassline into a MIDI track. Click or press Return to save a MIDI file.");
        setTooltip("Drag onto an empty MIDI clip slot or the arrangement of your instrument track. Click to save a .mid file instead.");
    }
    void paintButton(juce::Graphics& g, bool over, bool down) override
    {
        auto r = getLocalBounds().toFloat().reduced(.5f);
        if (down) r.translate(0, 1);
        g.setColour(over ? panel.brighter(.08f) : panel);
        g.fillRoundedRectangle(r, 7);
        g.setColour(hasKeyboardFocus(true) ? ink : border.brighter(.15f));
        g.drawRoundedRectangle(r, 7, hasKeyboardFocus(true) ? 1.5f : 1.0f);
        const float x = 20, y = r.getCentreY();
        g.setColour(muted);
        for (int i = 0; i < 3; ++i) for (int j = 0; j < 2; ++j)
            g.fillEllipse(x + j * 5.0f, y - 6 + i * 5.0f, 2, 2);
        labelText(g, "Drag MIDI", {39, 0, getWidth() - 62, getHeight()}, 15, ink, true);
        const float right = (float)getWidth() - 20;
        juce::Path arrow;
        arrow.startNewSubPath(right - 5, y + 4); arrow.lineTo(right + 3, y - 4);
        arrow.startNewSubPath(right - 3, y - 4); arrow.lineTo(right + 3, y - 4); arrow.lineTo(right + 3, y + 2);
        g.setColour(muted); g.strokePath(arrow, juce::PathStrokeType(1.4f));
    }

    void mouseDown(const juce::MouseEvent& e) override { dragged = false; Button::mouseDown(e); }
    void mouseDrag(const juce::MouseEvent& e) override
    {
        Button::mouseDrag(e);
        if (!dragged && e.getDistanceFromDragStart() >= 6) {
            dragged = true;
            if (onBeginDrag) onBeginDrag();
        }
    }
    void mouseUp(const juce::MouseEvent& e) override
    {
        if (dragged) setState(buttonNormal);
        else Button::mouseUp(e);
    }
    juce::String detail;
    std::function<void()> onBeginDrag;
private:
    bool dragged = false;
};
} // namespace

class UndertowEditor::Surface final : public juce::Component, private juce::Timer {
public:
    explicit Surface(UndertowProcessor& p) : processor(p), tooltips(this, 650)
    {
        setLookAndFeel(&theme); setOpaque(true); setSize(designWidth, designHeight);
        style.addItemList({"Deep House", "Rolling", "Offbeat", "Acid", "Disco", "Funk"}, 1);
        style.setTooltip("Choose the bassline character, then Generate to hear a new phrase.");
        const std::array<const char*, 12> keys{"C", "C#", "D", "Eb", "E", "F", "F#", "G", "Ab", "A", "Bb", "B"};
        for (int i = 0; i < 12; ++i) key.addItem(keys[(size_t)i], i + 1);
        scale.addItemList({"Minor", "Dorian", "Phrygian", "Major", "Minor pentatonic"}, 1);
        for (int bars : {1, 2, 4, 8}) length.addItem(juce::String(bars) + (bars == 1 ? " bar" : " bars"), bars);
        octave.addItemList({"Sub", "Bass", "Upper"}, 1);
        for (auto* box : {&style, &key, &scale, &length, &octave}) {
            addAndMakeVisible(*box); box->onChange = [this] { controlsChanged(); };
        }
        identify(style, "style", "Bassline style"); identify(key, "key", "Key"); identify(scale, "scale", "Scale");
        identify(length, "length", "Loop length"); identify(octave, "octave", "Register");
        key.setTooltip("Root note for new notes. Locked notes stay exactly as they are.");
        scale.setTooltip("Pitch collection for new notes. Choose the scale used by your track.");
        length.setTooltip("Loop length in 4/4. Generate applies the change. Locks beyond a shorter loop are removed.");
        octave.setTooltip("Sub starts at MIDI 24, Bass at MIDI 36, Upper at MIDI 48, plus the selected key.");

        const std::array<const char*, 4> macroNames{"Density", "Swing", "Note length", "Melody"};
        const std::array<const char*, 4> macroIDs{"density", "swing", "gate", "movement"};
        const std::array<const char*, 4> hints{
            "Fewer or more attacks around the repeating idea. Generate applies changes.",
            "Delays alternating sixteenths. 0% is straight; 100% is a 75:25 long-short pair.",
            "Shorter or longer notes, keeping the style's gaps between attacks.",
            "More pitch answers and short resolving notes. Lower values stay closer to the root."};
        for (int i = 0; i < 4; ++i) {
            auto& slider = macros[(size_t)i]; addAndMakeVisible(slider);
            identify(slider, macroIDs[(size_t)i], macroNames[(size_t)i]);
            slider.setSliderStyle(juce::Slider::LinearHorizontal);
            slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
            slider.setRange(0, 1, .01); slider.setScrollWheelEnabled(false);
            slider.setDoubleClickReturnValue(true, i == 0 ? .55 : (i == 1 ? .12 : (i == 2 ? .65 : .35)));
            slider.setTooltip(hints[(size_t)i]);
            slider.onValueChange = [this] { controlsChanged(); };
        }
        rootOnly.setButtonText("Root notes only"); rootAnchor.setButtonText("Start on root"); octaves.setButtonText("Octave jumps");
        identify(rootOnly, "root-only", "Root notes only"); identify(rootAnchor, "root-anchor", "Start on root");
        identify(octaves, "octave-jumps", "Octave jumps");
        rootOnly.setTooltip("Use only the root note. With Octave jumps on, its upper octave is allowed too. Locks stay exact.");
        rootAnchor.setTooltip("Put the first sounding note on the root without changing its rhythmic position. Locks take priority.");
        octaves.setTooltip("Allow upper-octave root notes as part of the phrase. Locks stay exact.");
        for (auto* toggle : {&rootOnly, &rootAnchor, &octaves}) {
            addAndMakeVisible(*toggle); toggle->onClick = [this] { controlsChanged(); };
        }
        addButton(more, "More controls", "Show timing and pitch controls.", [this] {
            expanded = !expanded; updateDisclosure();
            if (onDisclosure) onDisclosure(expanded);
            more.grabKeyboardFocus();
        });
        identify(more, "more-controls", "More controls"); more.getProperties().set("quiet", true); more.getProperties().set("disclosure", true);
        addButton(generate, "Generate", "Create a new bassline. Locked notes inside the loop stay exact.", [this] { generatePattern(false); });
        identify(generate, "generate", "Generate bassline"); generate.getProperties().set("primary", true);
        addButton(variation, "Change a few notes", "Keep the rhythm and most notes. Change a few pitches, lengths or accents; locked notes stay exact.", [this] { if (!processor.hasPendingSettings()) generatePattern(true); });
        identify(variation, "variation", "Change a few notes"); variation.getProperties().set("quiet", true);
        variation.getProperties().set("inlineAction", true);
        addButton(undo, "Undo", "Restore the previous phrase and controls.", [this] {
            processor.undo(); syncControls(); syncPattern(); status("Previous phrase restored.");
        });
        addButton(redo, "Redo", "Restore the phrase you just undid.", [this] {
            processor.redo(); syncControls(); syncPattern(); status("Phrase restored.");
        });
        identify(undo, "undo", "Undo"); identify(redo, "redo", "Redo");
        undo.getProperties().set("quiet", true); redo.getProperties().set("quiet", true);
        addButton(clearLocks, "Unlock all", "Allow every note to change again.", [this] {
            processor.clearLocks(); syncPattern(); status("All notes unlocked.");
        });
        identify(clearLocks, "unlock-all", "Unlock all notes"); clearLocks.getProperties().set("quiet", true);
        addButton(help, "Help", "Routing, MIDI export and note cleanup.", [this] { showMenu(); });
        identify(help, "help", "Help and MIDI actions"); help.getProperties().set("quiet", true);
        output.setButtonText("Live MIDI"); addAndMakeVisible(output); identify(output, "midi-output", "Live MIDI output");
        output.setTooltip("Send MIDI to your instrument while the DAW plays. Ableton VST3 uses two MIDI tracks; use the Max for Live device before Serum for one-track playback. Turn off after exporting to avoid doubled notes.");
        output.onClick = [this] { processor.setEnabled(output.getToggleState()); status(output.getToggleState() ? "Live MIDI on. Play your DAW to hear your instrument." : "Live MIDI off. Your exported clips can play."); };
        addAndMakeVisible(roll); identify(roll, "piano-roll", "Bassline piano roll");
        roll.onLock = [this](int i) { processor.toggleLock(i); syncPattern(); };
        addAndMakeVisible(drag); drag.setComponentID("midi-drag");
        drag.onClick = [this] { saveMidi(); }; drag.onBeginDrag = [this] { dragMidi(); };
        // A predictable keyboard path follows the visible composition workflow.
        int focus = 1;
        for (auto* c : std::initializer_list<juce::Component*>{&style, &key, &scale, &length, &more, &roll, &undo, &redo, &clearLocks,
             &macros[0], &macros[3], &macros[1], &macros[2], &octave, &rootOnly, &rootAnchor, &octaves,
             &generate, &variation, &drag, &output, &help}) c->setExplicitFocusOrder(focus++);
        updateDisclosure(); syncControls(); syncPattern(); startTimerHz(30);
    }
    ~Surface() override
    {
        stopTimer(); if (menuOpen) juce::PopupMenu::dismissAllActiveMenus(); chooser.reset(); setLookAndFeel(nullptr);
    }
    std::function<void(bool)> onDisclosure;
    void resized() override
    {
        help.setBounds(836, 22, 56, 32); output.setBounds(688, 22, 132, 32);
        style.setBounds(28, 92, 174, 34); key.setBounds(218, 92, 64, 34);
        scale.setBounds(298, 92, 174, 34); length.setBounds(488, 92, 100, 34);
        more.setBounds(742, 92, 150, 34);
        undo.setBounds(614, 146, 62, 30); redo.setBounds(680, 146, 62, 30); clearLocks.setBounds(770, 146, 122, 30);
        roll.setBounds(28, 182, 864, 200);
        macros[0].setBounds(28, 424, 400, 30); macros[3].setBounds(492, 424, 400, 30);
        macros[1].setBounds(28, 524, 190, 30); macros[2].setBounds(242, 524, 190, 30);
        octave.setBounds(456, 518, 132, 34);
        rootOnly.setBounds(622, 480, 270, 30); rootAnchor.setBounds(622, 518, 270, 30); octaves.setBounds(622, 556, 270, 30);
        const int actionY = expanded ? 616 : 476;
        generate.setBounds(28, actionY, 140, 38); variation.setBounds(174, actionY, 160, 38); drag.setBounds(738, actionY, 154, 38);
    }

    void paint(juce::Graphics& g) override
    {
        g.fillAll(background);
        g.setColour(accent);
        juce::Path logo;
        logo.startNewSubPath(28, 43); logo.lineTo(33, 43); logo.lineTo(37, 32); logo.lineTo(42, 52);
        logo.lineTo(47, 37); logo.lineTo(51, 43); logo.lineTo(57, 43);
        g.strokePath(logo, juce::PathStrokeType(1.7f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        labelText(g, JucePlugin_Name, {69, 20, 170, 38}, 26, ink, true);
        labelText(g, "by " + juce::String(JucePlugin_Manufacturer), {248, 28, 210, 24}, 13, muted);
        labelText(g, "Style", {28, 69, 174, 20}, 13, muted);
        labelText(g, "Key", {218, 69, 64, 20}, 13, muted);
        labelText(g, "Scale", {298, 69, 174, 20}, 13, muted);
        labelText(g, "Length", {488, 69, 100, 20}, 13, muted);
        labelText(g, "Bassline", {28, 147, 110, 28}, 15, ink, true);
        labelText(g, juce::String(cachedPattern.count) + " notes" + (lockedCount ? "  /  " + juce::String(lockedCount) + " locked" : ""),
                  {132, 148, 228, 28}, 13, muted);
        if (lockedCount == 0) labelText(g, "Click notes to lock", {398, 148, 198, 28}, 13, muted, false, juce::Justification::centredRight);
        g.setColour(border.withAlpha(.7f)); g.drawHorizontalLine(394, 28, 892);
        macroLabel(g, "Density", 0, 28, 402, 400); macroLabel(g, "Melody", 3, 492, 402, 400);
        if (expanded) {
            g.setColour(border.withAlpha(.7f)); g.drawHorizontalLine(467, 28, 892);
            macroLabel(g, "Swing", 1, 28, 494, 190); macroLabel(g, "Note length", 2, 242, 494, 190);
            labelText(g, "Register", {456, 494, 132, 22}, 13, muted);
        }
        const auto pending = processor.hasPendingSettings();
        juce::String message = statusMessage;
        if (message.isEmpty()) {
            if (pending) message = "Settings changed. Generate to apply.";
            else if (!processor.isEnabled()) message = "Live MIDI off";
            else if (processor.isHostPlaying()) message = "Sending MIDI to your instrument";
            else message = "Play your DAW to hear your instrument";
        }
        labelText(g, message, {28, getHeight() - 32, 704, 24}, 12.5f, pending && statusMessage.isEmpty() ? accent : muted);
        if (processor.hasHostPosition())
            labelText(g, juce::String(processor.getBpm(), 1) + " BPM", {770, getHeight() - 32, 122, 24}, 12.5f, muted, false, juce::Justification::centredRight);
    }
private:
    static void identify(juce::Component& c, const juce::String& id, const juce::String& name)
    { c.setComponentID(id); c.setName(name); c.setTitle(name); }
    void macroLabel(juce::Graphics& g, const juce::String& name, int i, int x, int y, int width)
    {
        labelText(g, name, {x, y, width - 62, 24}, 14, ink);
        labelText(g, juce::String(juce::roundToInt(macros[(size_t)i].getValue() * 100)) + "%", {x + width - 62, y, 62, 24}, 13, muted, false, juce::Justification::centredRight);
    }
    void updateDisclosure()
    {
        for (auto* c : std::initializer_list<juce::Component*>{&macros[1], &macros[2], &octave, &rootOnly, &rootAnchor, &octaves}) c->setVisible(expanded);
        more.setButtonText(expanded ? "Fewer controls" : "More controls");
        more.setTitle(expanded ? "Hide timing and pitch controls" : "Show timing and pitch controls");
        more.setToggleState(expanded, juce::dontSendNotification);
        resized(); repaint();
    }
    void addButton(juce::TextButton& b, const juce::String& text, const juce::String& hint, std::function<void()> fn)
    {
        addAndMakeVisible(b); b.setButtonText(text); b.setTitle(text); b.setTooltip(hint); b.onClick = std::move(fn);
    }
    void generatePattern(bool vary)
    {
        const auto old = processor.getPattern(); const auto beats = processor.getSettings().bars * 4.0;
        int cropped = 0;
        for (int i = 0; i < old.count; ++i) if (old.notes[(size_t)i].locked && old.notes[(size_t)i].start + old.notes[(size_t)i].duration > beats + 1.e-9) ++cropped;
        const bool applied = processor.hasPendingSettings();
        processor.generateNew(vary); syncPattern();
        status(cropped > 0 ? "Generated. " + juce::String(cropped) + " locked note(s) fell outside the shorter loop."
               : (vary && !applied ? "Phrase varied. Locked notes kept." : "New bassline generated."));
    }
    void controlsChanged()
    {
        if (updating) return;
        auto s = processor.getSettings();
        s.style = static_cast<undertow::Style>(style.getSelectedId() - 1);
        s.root = key.getSelectedId() - 1; s.scale = static_cast<undertow::Scale>(scale.getSelectedId() - 1);
        s.bars = length.getSelectedId(); s.octave = octave.getSelectedId() - 1;
        s.density = (float)macros[0].getValue(); s.swing = (float)macros[1].getValue();
        s.gate = (float)macros[2].getValue(); s.movement = (float)macros[3].getValue();
        s.rootOnly = rootOnly.getToggleState(); s.anchorRoot = rootAnchor.getToggleState(); s.octaveJumps = octaves.getToggleState();
        processor.setSettings(s); observedSettings = processor.getSettings(); variation.setEnabled(!processor.hasPendingSettings()); statusMessage.clear(); statusUntil = 0; repaint();
    }
    void syncControls()
    {
        const juce::ScopedValueSetter<bool> guard(updating, true);
        observedSettings = processor.getSettings(); const auto& s = observedSettings;
        key.setSelectedId(s.root + 1, juce::dontSendNotification); scale.setSelectedId((int)s.scale + 1, juce::dontSendNotification);
        style.setSelectedId((int)s.style + 1, juce::dontSendNotification);
        length.setSelectedId(s.bars, juce::dontSendNotification); octave.setSelectedId(s.octave + 1, juce::dontSendNotification);
        macros[0].setValue(s.density, juce::dontSendNotification); macros[1].setValue(s.swing, juce::dontSendNotification);
        macros[2].setValue(s.gate, juce::dontSendNotification); macros[3].setValue(s.movement, juce::dontSendNotification);
        rootOnly.setToggleState(s.rootOnly, juce::dontSendNotification); rootAnchor.setToggleState(s.anchorRoot, juce::dontSendNotification);
        octaves.setToggleState(s.octaveJumps, juce::dontSendNotification); output.setToggleState(processor.isEnabled(), juce::dontSendNotification);
        variation.setEnabled(!processor.hasPendingSettings());
    }
    void syncPattern()
    {
        cachedRevision = processor.getRevision(); cachedPattern = processor.getPattern(); lockedCount = 0;
        for (int i = 0; i < cachedPattern.count; ++i) if (cachedPattern.notes[(size_t)i].locked) ++lockedCount;
        roll.setPattern(cachedPattern); clearLocks.setVisible(lockedCount > 0);
        undo.setEnabled(processor.canUndo()); redo.setEnabled(processor.canRedo());
        drag.setEnabled(cachedPattern.count > 0); variation.setEnabled(cachedPattern.count > 0 && !processor.hasPendingSettings()); repaint();
    }
    void timerCallback() override
    {
        if (!sameSettings(processor.getSettings(), observedSettings)) syncControls();
        if (cachedRevision != processor.getRevision()) syncPattern();
        output.setToggleState(processor.isEnabled(), juce::dontSendNotification);
        roll.setPlayhead(processor.isHostPlaying() && processor.isEnabled() ? processor.getPlayheadBeat() : -1);
        if (statusUntil > 0 && juce::Time::getMillisecondCounterHiRes() > statusUntil) { statusMessage.clear(); statusUntil = 0; }
        repaint(28, getHeight() - 32, 864, 24);
    }
    void status(const juce::String& message)
    { statusMessage = message; statusUntil = juce::Time::getMillisecondCounterHiRes() + 6000; repaint(); }
    void dragMidi()
    {
        const auto file = processor.createMidiDragFile();
        if (!file.existsAsFile()) { status("Could not create MIDI. Use Help > Save MIDI to try again."); return; }
        if (!juce::DragAndDropContainer::performExternalDragDropOfFiles({file.getFullPathName()}, false, &drag))
            status("Drag unavailable. Click Drag MIDI to save a file instead.");
        else status("Drop MIDI onto your instrument track. Turn Live MIDI off to play the clip.");
    }
    void saveMidi()
    {
        if (chooser) return;
        const auto suggested = juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
            .getChildFile("Submotif-" + juce::String((juce::int64)cachedPattern.seed) + ".mid");
        chooser = std::make_unique<juce::FileChooser>("Save your bassline", suggested, "*.mid", true);
        const juce::Component::SafePointer<Surface> safe(this);
        chooser->launchAsync(juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles | juce::FileBrowserComponent::warnAboutOverwriting,
            [safe](const juce::FileChooser& fc) {
                if (safe == nullptr) return;
                const auto target = fc.getResult();
                if (target != juce::File{}) safe->status(safe->processor.exportMidiTo(target) ? "Saved " + target.getFileName() : "Could not save MIDI. Choose a writable folder.");
                juce::MessageManager::callAsync([safe] { if (safe != nullptr) safe->chooser.reset(); });
            });
    }
    void showMenu()
    {
        juce::PopupMenu menu; menu.setLookAndFeel(&theme);
        menu.addItem(1, "Connect your instrument"); menu.addItem(2, "Save MIDI...");
        menu.addSeparator(); menu.addItem(3, "Stop notes");
        menu.addSeparator(); menu.addItem(4, "About SUBMOTIF");
        menuOpen = true;
        const juce::Component::SafePointer<Surface> safe(this);
        menu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(&help).withMinimumWidth(238), [safe](int choice) {
            if (safe == nullptr) return;
            safe->menuOpen = false;
            if (choice == 1) safe->showRouting();
            if (choice == 2) safe->saveMidi();
            if (choice == 3) { safe->processor.panic(); safe->status("Notes released. Switch Live MIDI off to stop generation."); }
            if (choice == 4) juce::AlertWindow::showMessageBoxAsync(juce::MessageBoxIconType::InfoIcon, JucePlugin_Name,
                "by " + juce::String(JucePlugin_Manufacturer) + "\nMIDI bassline generator · Version " + juce::String(JucePlugin_VersionString) + "\n\nGenerates notes for your instrument. Every Generate constructs a fresh phrase from weighted musical choices.\n\nClick notes to lock them. Arrow keys select a note; Space toggles its lock. Double-click a slider to reset it.\n\nMIDI patterns use 4/4.", "Close", safe.getComponent());
        });
    }
    void showRouting()
    {
        juce::AlertWindow::showMessageBoxAsync(juce::MessageBoxIconType::InfoIcon, "Connect your instrument",
            "Your instrument supplies the sound.\n\n"
            "Ableton Live · one track\n"
            "Load Submotif.amxd from the Max for Live edition before Serum or another instrument. Enable Live MIDI and start Live's transport.\n\n"
            "Chain: Submotif MIDI device > Serum > audio effects\n\n"
            "Ableton Live · VST3 edition\n"
            "Live loads this VST3 as an instrument. Dropping it onto Serum can replace Serum. Use two MIDI tracks:\n"
            "1. Put SUBMOTIF VST3 on its own MIDI track and Serum on a second.\n"
            "2. On Serum's track, select SUBMOTIF's track under MIDI From, then SUBMOTIF in the lower chooser.\n"
            "3. Set Monitor to In, enable Live MIDI here, and start playback.\n\n"
            "To keep the phrase, drag MIDI onto Serum's track. Turn Live MIDI off here and set that track's Monitor to Auto to play the clip.\n\n"
            "Other DAWs: route this plug-in's MIDI output to your instrument using your host's MIDI routing.", "Got it", this);
    }
    UndertowProcessor& processor;
    Theme theme;
    juce::TooltipWindow tooltips;
    juce::ComboBox style, key, scale, length, octave;
    std::array<juce::Slider, 4> macros;
    juce::ToggleButton rootOnly, rootAnchor, octaves, output;
    juce::TextButton generate, variation, undo, redo, clearLocks, more, help;
    PianoRoll roll;
    MidiDragButton drag;
    std::unique_ptr<juce::FileChooser> chooser;
    undertow::Settings observedSettings;
    undertow::Pattern cachedPattern;
    std::uint64_t cachedRevision = 0;
    int lockedCount = 0;
    bool updating = false, expanded = false, menuOpen = false;
    juce::String statusMessage;
    double statusUntil = 0;
};

UndertowEditor::UndertowEditor(UndertowProcessor& p) : juce::AudioProcessorEditor(p), surface(std::make_unique<Surface>(p))
{
    setOpaque(true);
    addAndMakeVisible(*surface);
    surface->onDisclosure = [this](bool expanded) { setDetailsExpanded(expanded); };
    setResizable(true, true);
    setResizeLimits(820, juce::roundToInt(820.0 * designHeight / designWidth), designWidth * 3 / 2, designHeight * 3 / 2);
    if (auto* constrainer = getConstrainer()) constrainer->setFixedAspectRatio((double)designWidth / designHeight);
    setSize(designWidth, designHeight);
}

UndertowEditor::~UndertowEditor() = default;

void UndertowEditor::resized()
{
    const auto scaleFactor = (float)getWidth() / (float)designWidth;
    surface->setTransform(juce::AffineTransform::scale(scaleFactor));
    surface->setBounds(0, 0, designWidth, detailsExpanded ? expandedHeight : designHeight);
}

void UndertowEditor::setDetailsExpanded(bool expanded)
{
    detailsExpanded = expanded;
    const int height = expanded ? expandedHeight : designHeight;
    int width = getWidth();
    // Fit the expanded editor inside the current display's usable area.
    if (expanded) if (const auto* display = juce::Desktop::getInstance().getDisplays().getDisplayForRect(getScreenBounds())) {
        const int fit = (int)std::floor(juce::jmin(display->userBounds.getWidth() - 48,
                                  (display->userBounds.getHeight() - 64) * designWidth / height));
        width = juce::jlimit(820, designWidth * 3 / 2, juce::jmin(width, fit));
    }
    if (auto* constrainer = getConstrainer()) constrainer->setFixedAspectRatio((double)designWidth / height);
    setResizeLimits(820, juce::roundToInt(820.0 * height / designWidth), designWidth * 3 / 2, height * 3 / 2);
    setSize(width, juce::roundToInt((double)width * height / designWidth));
}

void UndertowEditor::paint(juce::Graphics& g) { g.fillAll(background); }
