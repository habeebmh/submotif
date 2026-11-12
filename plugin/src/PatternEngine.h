#pragma once
#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace undertow {
constexpr int maxNotes = 128;
enum class Scale : int { minor, dorian, phrygian, major, minorPentatonic };
enum class Style : int { deep, rolling, offbeat, acid, disco, funk };
struct Settings {
    int root = 0;
    Scale scale = Scale::minor;
    Style style = Style::deep;
    int bars = 1;
    int octave = 1; // Ableton octave convention: C1 = MIDI 36.
    float density = 0.55f;
    float swing = 0.12f;
    float gate = 0.65f;
    float movement = 0.35f;
    bool rootOnly = false;
    bool anchorRoot = true;
    bool octaveJumps = true;
};
struct Note {
    double start = 0.0, duration = 0.25;
    int pitch = 36, velocity = 100;
    bool locked = false;
};
struct Pattern {
    std::array<Note, maxNotes> notes{};
    int count = 0;
    double lengthBeats = 8.0;
    std::uint32_t seed = 1;
};
Settings sanitise(Settings);
bool isValid(const Pattern&) noexcept;
// Transactional whole-phrase shift: no clipping, wrapping, or partial changes.
bool transposeOctaves(Pattern&, int octaves) noexcept;
Pattern generate(const Settings&, std::uint32_t seed, const Pattern* previous = nullptr, bool variation = false);
std::string noteName(int pitch);
std::vector<std::uint8_t> midiFile(const Pattern&);

struct MidiEvent { int sample = 0, pitch = 0, velocity = 0; bool on = false; };
struct EventBuffer {
    std::array<MidiEvent, 4096> events{};
    int count = 0;
};
class Scheduler {
public:
    const EventBuffer& process(const Pattern&, double ppq, double bpm, double sampleRate,
                              int samples, bool playing, bool enabled, std::uint64_t revision) noexcept;
    void reset() noexcept;
private:
    EventBuffer output;
    std::array<bool, 128> active{};
    bool wasPlaying = false;
    double expectedPpq = 0.0;
    std::uint64_t lastRevision = 0;
};
} // namespace undertow
