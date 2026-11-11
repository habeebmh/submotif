#include "PatternEngine.h"
#include <algorithm>
#include <cmath>
#include <limits>

namespace undertow {
namespace {
constexpr double epsilon = 1.e-9;
constexpr int ppqResolution = 960;
struct Random {
    explicit Random(std::uint32_t seed) : value(seed == 0 ? 0x9e3779b9u : seed) {}
    std::uint32_t value;
    std::uint32_t next() noexcept {
        value ^= value << 13; value ^= value >> 17; value ^= value << 5;
        return value;
    }
    double unit() noexcept { return static_cast<double>(next() >> 8) / 16777216.0; }
    int integer(int count) noexcept { return static_cast<int>(unit() * count); }
};
constexpr std::array<std::array<int, 7>, 5> intervals{{
    {{0,2,3,5,7,8,10}}, {{0,2,3,5,7,9,10}}, {{0,1,3,5,7,8,10}},
    {{0,2,4,5,7,9,11}}, {{0,3,5,7,10,0,0}}
}};
int degreeCount(Scale scale) noexcept { return scale == Scale::minorPentatonic ? 5 : 7; }
float finiteUnit(float value, float fallback) noexcept {
    return std::isfinite(value) ? std::clamp(value, 0.f, 1.f) : fallback;
}
bool belongsToScale(int pitch, const Settings& s) noexcept {
    const auto& set = intervals[static_cast<int>(s.scale)];
    const int relative = ((pitch - s.root) % 12 + 12) % 12;
    return std::find(set.begin(), set.begin() + degreeCount(s.scale), relative)
        != set.begin() + degreeCount(s.scale);
}
bool intersects(const Note& a, const Note& b) noexcept {
    return a.start < b.start + b.duration - epsilon
        && b.start < a.start + a.duration - epsilon;
}

// The style is a probability model over metric positions, not a bank of stored
// patterns. Each candidate onset is drawn again on every Generate. The numeric
// weights below are our musical design choices; see docs/MUSIC_THEORY.md for the
// distinction between those choices and the research that motivated them.
double onsetWeight(Style style, int step, int previousStep, float density) noexcept {
    const bool beat = step % 4 == 0;
    const bool eighthOffbeat = step % 4 == 2;
    const bool sixteenthOffbeat = step % 2 == 1;
    const bool lastBeat = step % 16 >= 12;
    double weight = .1;
    switch (style) {
        case Style::deep:
            weight = beat ? .58 : (eighthOffbeat ? .72 : .14);
            if (step % 16 == 0) weight += .22;
            if (step % 8 == 3) weight += .26;
            break;
        case Style::rolling:
            weight = beat ? .80 : (eighthOffbeat ? .68 : .43);
            if (previousStep == step - 1 && sixteenthOffbeat) weight += .13;
            break;
        case Style::offbeat:
            weight = eighthOffbeat ? .98 : (sixteenthOffbeat ? .035 : .025);
            if (lastBeat && step % 4 == 3) weight += .08;
            break;
        case Style::acid:
            weight = beat ? .72 : (eighthOffbeat ? .61 : .49);
            if (step % 16 == 0) weight += .15;
            break;
        case Style::disco:
            weight = beat ? .85 : (eighthOffbeat ? .82 : .09);
            if (lastBeat && sixteenthOffbeat) weight += .15;
            break;
        case Style::funk:
            weight = beat ? .48 : (eighthOffbeat ? .58 : .37);
            if (step % 4 == 3) weight += .27; // anticipation of a beat
            if (step % 16 == 0) weight += .35;
            break;
    }
    // Low density preserves the strongest stylistic positions, while high
    // density adds possibilities without erasing the metric hierarchy.
    weight *= .20 + 1.02 * density;
    if (step - previousStep >= 8 && style != Style::offbeat) weight += .14;
    return std::clamp(weight, 0.0, .985);
}

int selectPitch(const Settings& s, Random& random, int step, int& lastDegree) noexcept {
    const int base = (s.octave + 2) * 12 + s.root;
    const auto& set = intervals[static_cast<int>(s.scale)];
    const int count = degreeCount(s.scale);
    int degree = 0;
    if (!s.rootOnly) {
        const double tonicWeight = .82 - .52 * s.movement;
        if (random.unit() >= tonicWeight) {
            if (s.style == Style::acid || random.unit() < .55) {
                // Nearby scale degrees keep a coherent monophonic contour.
                const int direction = random.unit() < .5 ? -1 : 1;
                degree = std::clamp(lastDegree + direction, 0, count - 1);
            } else {
                // Weight stable tonic/third/fifth positions over remote tones.
                std::array<double, 7> weights{};
                double sum = 0;
                for (int i = 0; i < count; ++i) {
                    double weight = .35 + .75 * s.movement;
                    if (set[i] == 0) weight += 2.0;
                    if (set[i] == 7) weight += s.style == Style::disco ? 3.3 : 2.1;
                    if (set[i] == 3 || set[i] == 4) weight += 1.15;
                    if (set[i] >= 9 && s.style == Style::funk) weight += 1.0;
                    if (step % 16 >= 12 && i == 1) weight += 1.2 * s.movement;
                    weights[i] = weight; sum += weight;
                }
                double choice = random.unit() * sum;
                for (degree = 0; degree < count - 1; ++degree)
                    if ((choice -= weights[degree]) < 0) break;
            }
        }
    }
    lastDegree = degree;
    int pitch = base + set[degree];
    if (s.octaveJumps) {
        double octaveChance = .04 + .15 * s.movement;
        if (s.style == Style::disco && step % 4 == 2) octaveChance += .62;
        if (s.style == Style::funk && step % 4 == 3) octaveChance += .20;
        if (random.unit() < octaveChance && pitch + 12 <= 71) pitch += 12;
    }
    return pitch;
}

void appendBE32(std::vector<std::uint8_t>& bytes, std::uint32_t value) {
    for (int shift = 24; shift >= 0; shift -= 8)
        bytes.push_back(static_cast<std::uint8_t>((value >> shift) & 255));
}
void appendVLQ(std::vector<std::uint8_t>& bytes, std::uint32_t value) {
    std::array<std::uint8_t, 4> digits{};
    int count = 0;
    digits[count++] = static_cast<std::uint8_t>(value & 127);
    while ((value >>= 7) != 0) digits[count++] = static_cast<std::uint8_t>((value & 127) | 128);
    while (count > 0) bytes.push_back(digits[--count]);
}
} // namespace

Settings sanitise(Settings s) {
    s.root = std::clamp(s.root, 0, 11);
    s.scale = static_cast<Scale>(std::clamp(static_cast<int>(s.scale), 0, 4));
    s.style = static_cast<Style>(std::clamp(static_cast<int>(s.style), 0, 5));
    s.bars = s.bars >= 8 ? 8 : s.bars >= 4 ? 4 : s.bars >= 2 ? 2 : 1;
    s.octave = std::clamp(s.octave, 0, 2);
    s.density = finiteUnit(s.density, .55f);
    s.swing = finiteUnit(s.swing, .12f);
    s.gate = finiteUnit(s.gate, .65f);
    s.movement = finiteUnit(s.movement, .35f);
    return s;
}

bool isValid(const Pattern& p) noexcept {
    if (p.count < 0 || p.count > maxNotes || !std::isfinite(p.lengthBeats)
        || (p.lengthBeats != 4.0 && p.lengthBeats != 8.0
            && p.lengthBeats != 16.0 && p.lengthBeats != 32.0)) return false;
    double previousEnd = 0;
    for (int i = 0; i < p.count; ++i) {
        const auto& n = p.notes[i];
        if (!std::isfinite(n.start) || !std::isfinite(n.duration)
            || n.start < 0 || n.duration <= 0 || n.start + n.duration > p.lengthBeats + epsilon
            || n.pitch < 0 || n.pitch > 127 || n.velocity < 1 || n.velocity > 127
            || n.start < previousEnd - epsilon) return false;
        previousEnd = n.start + n.duration;
    }
    return true;
}

Pattern generate(const Settings& input, std::uint32_t seed, const Pattern* previous, bool variation) {
    const Settings s = sanitise(input);
    Random random(seed);
    Pattern result;
    result.lengthBeats = s.bars * 4.0;
    result.seed = seed;
    if (previous != nullptr && !isValid(*previous)) previous = nullptr;
    std::vector<Note> locked, candidates;
    locked.reserve(maxNotes); candidates.reserve(maxNotes * 2);
    if (previous != nullptr) {
        for (int i = 0; i < previous->count; ++i) {
            const auto& n = previous->notes[i];
            if (n.locked && n.start + n.duration <= result.lengthBeats + epsilon) locked.push_back(n);
        }
    }
    // Variations keep selected unpinned onsets/pitches too; only explicitly locked
    // notes promise exact preservation. Settings changes still apply to new notes.
    if (variation && previous != nullptr) {
        for (int i = 0; i < previous->count; ++i) {
            const auto& n = previous->notes[i];
            const int base = (s.octave + 2) * 12 + s.root;
            const bool compatible = belongsToScale(n.pitch, s)
                && (!s.rootOnly || n.pitch % 12 == s.root)
                && n.pitch >= base && n.pitch <= 71
                && (s.octaveJumps || n.pitch < base + 12)
                && n.start + n.duration <= result.lengthBeats + epsilon;
            if (!n.locked && compatible && random.unit() < .76) candidates.push_back(n);
        }
    }
    int previousStep = -8, lastDegree = 0;
    for (int step = 0; step < s.bars * 16; ++step) {
        const double start = step * .25 + (step % 2 != 0 ? .125 * s.swing : 0);
        const double weight = onsetWeight(s.style, step, previousStep, s.density);
        const bool chosen = random.unit() < weight;
        if (!chosen) continue;
        previousStep = step;
        Note n;
        n.start = start;
        n.pitch = selectPitch(s, random, step, lastDegree);
        const bool strong = step % 4 == 0;
        const bool accentedOffbeat = step % 4 == 2 && (s.style == Style::disco || s.style == Style::offbeat);
        int velocity = 78 + random.integer(21) + (strong ? 14 : 0) + (accentedOffbeat ? 8 : 0);
        if (s.style == Style::funk && step % 4 == 1) velocity -= 16;
        if (s.style == Style::acid && random.unit() < .28) velocity += 16;
        n.velocity = std::clamp(velocity, 40, 124);
        const double baseLength = s.style == Style::deep ? .70 : (s.style == Style::offbeat ? .43 : .38);
        n.duration = std::clamp(baseLength * (.22 + s.gate * 1.55) * (.8 + random.unit() * .4), .045, 1.5);
        if (s.style == Style::acid && random.unit() < .18 * s.gate) n.duration *= 1.6;
        // Retained variation onsets win over newly drawn notes at the same point.
        if (std::none_of(candidates.begin(), candidates.end(), [&](const Note& kept) {
                return std::abs(kept.start - n.start) < .04;
            })) candidates.push_back(n);
    }
    if (candidates.empty() && locked.empty()) {
        const double onset = s.style == Style::offbeat ? .5 : 0;
        candidates.push_back({onset, .16 + .54 * s.gate,
            (s.octave + 2) * 12 + s.root, 104, false});
    }
    std::sort(candidates.begin(), candidates.end(), [](const Note& a, const Note& b) {return a.start < b.start;});
    // Locks take priority: truncate a new note before a lock, and never add a new
    // onset inside it. Locked notes themselves are never shortened or repitched.
    std::vector<Note> assembled = locked;
    assembled.reserve(maxNotes);
    for (auto n : candidates) {
        if (assembled.size() >= maxNotes) break;
        n.duration = std::min(n.duration, result.lengthBeats - n.start);
        bool reject = false;
        for (const auto& kept : locked) {
            if (n.start >= kept.start - epsilon && n.start < kept.start + kept.duration - epsilon) {
                reject = true; break;
            }
            if (intersects(n, kept)) n.duration = std::min(n.duration, kept.start - n.start);
        }
        if (!reject && n.duration >= .0011) assembled.push_back(n);
    }
    std::sort(assembled.begin(), assembled.end(), [](const Note& a, const Note& b) {return a.start < b.start;});
    for (std::size_t i = 0; i < assembled.size(); ++i) {
        auto n = assembled[i];
        if (!n.locked && i + 1 < assembled.size())
            n.duration = std::min(n.duration, assembled[i + 1].start - n.start);
        if (n.duration <= epsilon) continue;
        if (result.count == 0 && s.anchorRoot && !n.locked)
            n.pitch = (s.octave + 2) * 12 + s.root;
        result.notes[result.count++] = n;
    }
    return result;
}

std::string noteName(int pitch) {
    static constexpr const char* names[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
    pitch = std::clamp(pitch, 0, 127);
    return std::string(names[pitch % 12]) + std::to_string(pitch / 12 - 2);
}

std::vector<std::uint8_t> midiFile(const Pattern& p) {
    if (!isValid(p)) return {};
    struct Event { std::uint32_t tick; bool on; int pitch, velocity; };
    std::vector<Event> events;
    events.reserve(static_cast<std::size_t>(p.count * 2));
    const auto endTick = static_cast<std::uint32_t>(std::llround(p.lengthBeats * ppqResolution));
    if (endTick == 0) return {};
    for (int i = 0; i < p.count; ++i) {
        const auto& n = p.notes[i];
        const auto start = static_cast<std::uint32_t>(std::llround(n.start * ppqResolution));
        auto end = static_cast<std::uint32_t>(std::llround((n.start + n.duration) * ppqResolution));
        end = std::min(endTick, std::max(start + 1, end));
        if (end <= start) return {};
        if (i + 1 < p.count && end > static_cast<std::uint32_t>(std::llround(p.notes[i + 1].start * ppqResolution))) return {};
        events.push_back({start, true, n.pitch, n.velocity});
        events.push_back({end, false, n.pitch, 0});
    }
    std::sort(events.begin(), events.end(), [](const Event& a, const Event& b) {
        if (a.tick != b.tick) return a.tick < b.tick;
        return a.on < b.on; // note-offs precede any new onset at the same tick
    });
    const std::string trackName = "Undertow Bassline";
    std::vector<std::uint8_t> track{0, 255, 3, static_cast<std::uint8_t>(trackName.size())};
    track.insert(track.end(), trackName.begin(), trackName.end());
    track.insert(track.end(), {0, 255, 0x58, 4, 4, 2, 24, 8});
    std::uint32_t currentTick = 0;
    for (const auto& event : events) {
        appendVLQ(track, event.tick - currentTick);
        track.push_back(event.on ? 0x90 : 0x80);
        track.push_back(static_cast<std::uint8_t>(event.pitch));
        track.push_back(static_cast<std::uint8_t>(event.velocity));
        currentTick = event.tick;
    }
    appendVLQ(track, endTick - currentTick);
    track.insert(track.end(), {255, 0x2f, 0});
    std::vector<std::uint8_t> file{'M','T','h','d',0,0,0,6,0,0,0,1,3,192,'M','T','r','k'};
    appendBE32(file, static_cast<std::uint32_t>(track.size()));
    file.insert(file.end(), track.begin(), track.end());
    return file;
}

const EventBuffer& Scheduler::process(const Pattern& p, double ppq, double bpm, double sampleRate,
                                     int samples, bool playing, bool enabled, std::uint64_t revision) noexcept {
    output.count = 0;
    if (samples <= 0) return output;
    auto flush = [&]() noexcept {
        for (int pitch = 0; pitch < 128; ++pitch) {
            if (active[pitch]) output.events[output.count++] = {0, pitch, 0, false};
            active[pitch] = false;
        }
    };
    const bool validTiming = std::isfinite(ppq) && std::abs(ppq) < 1.e12
        && std::isfinite(bpm) && bpm > 0 && bpm <= 1000
        && std::isfinite(sampleRate) && sampleRate > 0;
    const bool running = playing && enabled && validTiming && isValid(p);
    if (!running) {
        flush(); wasPlaying = false; lastRevision = revision;
        return output;
    }
    const double beatsPerSample = bpm / (60.0 * sampleRate);
    const double end = ppq + samples * beatsPerSample;
    if (!std::isfinite(end) || !std::isfinite(beatsPerSample) || beatsPerSample <= 0) {
        flush(); wasPlaying = false;
        return output;
    }
    const bool discontinuity = !wasPlaying || revision != lastRevision
        || std::abs(ppq - expectedPpq) > std::max(1.e-8, beatsPerSample * .25);
    if (discontinuity) flush();
    const int prefixCount = output.count;
    bool overflow = false;
    auto append = [&](int sample, int pitch, int velocity, bool on) noexcept {
        if (sample < 0 || sample >= samples) return;
        if (output.count == static_cast<int>(output.events.size())) {overflow = true; return;}
        output.events[output.count++] = {sample, pitch, velocity, on};
    };
    // Ceil quantises an event to the first sample at or after its musical time.
    // Including the immediately previous sample lets the next block own an event
    // just before its raw PPQ boundary, avoiding partition-dependent early notes.
    auto offset = [&](double beat) noexcept -> double {
        return std::ceil((beat - ppq) / beatsPerSample - 1.e-7);
    };
    const double startCycle = std::floor((ppq - beatsPerSample) / p.lengthBeats);
    const double endCycle = std::floor(end / p.lengthBeats);
    if (!std::isfinite(startCycle) || !std::isfinite(endCycle)
        || endCycle < startCycle || endCycle - startCycle > 4096) {
        // Malformed/extreme host blocks cannot make the real-time callback loop
        // without bound. Silence is safer than leaving an incomplete note stream.
        flush(); wasPlaying = false;
        return output;
    }
    for (double cycle = startCycle; cycle <= endCycle && !overflow; cycle += 1.0) {
        const double cycleStart = cycle * p.lengthBeats;
        for (int i = 0; i < p.count && !overflow; ++i) {
            const auto& n = p.notes[i];
            const double onSample = offset(cycleStart + n.start);
            const double offSample = offset(cycleStart + n.start + n.duration);
            // Subsample-duration notes that quantise to one position are silent;
            // emitting off-before-on would otherwise leave a hanging note.
            if (onSample == offSample) continue;
            if (discontinuity && onSample < 0 && offSample > 0)
                append(0, n.pitch, n.velocity, true);
            if (onSample >= 0 && onSample < samples)
                append(static_cast<int>(onSample), n.pitch, n.velocity, true);
            if (offSample >= 0 && offSample < samples)
                append(static_cast<int>(offSample), n.pitch, 0, false);
        }
    }
    if (overflow) {
        output.count = prefixCount;
        flush(); wasPlaying = false;
        return output;
    }
    std::sort(output.events.begin() + prefixCount, output.events.begin() + output.count,
        [](const MidiEvent& a, const MidiEvent& b) noexcept {
            if (a.sample != b.sample) return a.sample < b.sample;
            if (a.on != b.on) return a.on < b.on;
            return a.pitch < b.pitch;
        });
    int written = prefixCount;
    for (int i = prefixCount; i < output.count; ++i) {
        const auto event = output.events[i];
        if (event.on == active[event.pitch]) continue;
        active[event.pitch] = event.on;
        output.events[written++] = event;
    }
    output.count = written;
    wasPlaying = true;
    expectedPpq = end;
    lastRevision = revision;
    return output;
}

void Scheduler::reset() noexcept {
    output.count = 0;
    active.fill(false);
    wasPlaying = false;
    expectedPpq = 0;
    lastRevision = 0;
}
} // namespace undertow
