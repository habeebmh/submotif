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

// Composition is hierarchical: a fresh weighted rhythm and harmonic plan make
// one motif; the phrase repeats it with a small number of ending responses.
// Numeric profiles are design choices, not empirical genre measurements.
int basePitch(const Settings& s) noexcept { return (s.octave + 2) * 12 + s.root; }
int third(const Settings& s) noexcept { return s.scale == Scale::major ? 4 : 3; }
bool triadic(int pitch, const Settings& s) noexcept {
    const int pc = ((pitch - s.root) % 12 + 12) % 12;
    return pc == 0 || pc == third(s) || pc == 7;
}
int gridStep(double start) noexcept { return static_cast<int>(std::floor(start * 4.0 + 1.e-7)); }
double onset(int step, const Settings& s) noexcept {
    return step * .25 + (step % 2 ? .125 * static_cast<double>(s.swing) : 0.0);
}
int roundCount(double n) noexcept { return static_cast<int>(std::floor(n + .5)); }

// Produce the entire weighted ordering before taking its prefix. Density adds
// attacks to the same seed's identity, rather than rerolling each grid position.
std::vector<int> weightedOrder(std::array<double, 16> weights, Random& random) {
    std::vector<int> order;
    for (int pick = 0; pick < 16; ++pick) {
        double sum = 0;
        for (double w : weights) sum += w;
        if (sum <= 0) break;
        double draw = random.unit() * sum;
        int selected = -1;
        for (int i = 0; i < 16; ++i) {
            if (weights[i] <= 0) continue;
            selected = i;
            if ((draw -= weights[i]) < 0) break;
        }
        order.push_back(selected); weights[selected] = 0;
    }
    return order;
}
std::array<bool, 16> makeRhythm(const Settings& s, Random& random) {
    std::array<bool, 16> mask{};
    std::array<double, 16> weights{};
    int budget = 0;
    if (s.style == Style::offbeat) {
        mask[2] = mask[10] = true;
        weights[6] = weights[14] = 1;
        const auto support = weightedOrder(weights, random);
        const int extras = std::min(2, roundCount(4.0 * s.density));
        for (int i = 0; i < extras; ++i) mask[support[i]] = true;
        weights.fill(0);
        // A high-density pickup belongs to an existing offbeat pulse.
        for (int step : {6, 10, 14}) if (mask[step]) weights[step - 1] = step >= 10 ? 2.0 : .5;
        const auto pickups = weightedOrder(weights, random);
        const int count = s.density > .75 ? roundCount((s.density - .75) * 8.0) : 0;
        for (int i = 0; i < count; ++i) mask[pickups[i]] = true;
        return mask;
    }
    if (s.style == Style::rolling) {
        const int arrival = random.unit() < .65 ? 0 : 2;
        const int reply = arrival == 0 ? (random.unit() < .55 ? 3 : 6) : (random.unit() < .5 ? 5 : 6);
        mask[arrival] = mask[reply] = true;
        budget = 2 + roundCount(3.0 * s.density);
        for (int step = arrival + 1; step < 8; ++step)
            if (!mask[step]) weights[step] = step == arrival + 1 || step == reply + 1 ? 1.8 : (step % 2 ? .8 : 1.2);
        const auto order = weightedOrder(weights, random);
        for (int i = 0; i < budget - 2; ++i) mask[order[i]] = true;
        for (int i = 0; i < 8; ++i) mask[i + 8] = mask[i];
        return mask;
    }
    int arrival = 0, reply = 8;
    if (s.style == Style::deep || s.style == Style::funk) {
        const double phase = random.unit();
        arrival = phase < (s.style == Style::deep ? .65 : .5) ? 0 : phase < .8 ? 2 : 3;
        // An anticipation replaces the later beat; it is not an extra random hit.
        reply = random.unit() < .4 ? 7 : arrival + 8;
    }
    mask[arrival] = mask[reply] = true;
    if (s.style == Style::disco) { mask[2] = mask[10] = true; budget = 4 + roundCount(8.0 * s.density); }
    else if (s.style == Style::deep) budget = 2 + roundCount(6.0 * s.density);
    else if (s.style == Style::acid) budget = 4 + roundCount(7.0 * s.density);
    else budget = 3 + roundCount(6.0 * s.density);
    for (int step = 0; step < 16; ++step) {
        if (mask[step] || step < arrival || (reply == 7 && step == 8)
            || (reply > 8 && step >= 8 && step < reply)) continue;
        const bool beat = step % 4 == 0, eighth = step % 4 == 2;
        if (s.style == Style::deep) weights[step] = eighth ? 2.0 : (step % 4 == 3 ? 1.5 : (beat ? .55 : .35));
        else if (s.style == Style::acid) weights[step] = step % 2 ? 1.4 : 1.0;
        else if (s.style == Style::disco) weights[step] = step % 2 ? .18 : 5.0;
        else weights[step] = step % 4 == 3 ? 2.5 : (eighth ? 1.3 : (beat ? .5 : .8));
        if (step >= 12 && s.style != Style::disco) weights[step] *= 1.15;
        if (s.style == Style::deep || s.style == Style::funk) {
            // Responses are related to the selected arrivals, not just the grid.
            if (step == arrival + 2 || step == reply + 2) weights[step] *= 2.0;
            if (step == reply - 1) weights[step] *= 1.5;
        }
    }
    const auto order = weightedOrder(weights, random);
    const int mandatory = s.style == Style::disco ? 4 : 2;
    for (int i = 0; i < budget - mandatory; ++i) mask[order[i]] = true;
    return mask;
}

int nearestTriad(int pitch, const Settings& s) noexcept {
    const int base = basePitch(s);
    std::array<int, 4> choices{base, base + third(s), base + 7, base + 12};
    int best = base, distance = std::abs(pitch - base);
    for (int p : choices) {
        if (p > 71 || (s.rootOnly && p % 12 != s.root) || (!s.octaveJumps && p == base + 12)) continue;
        if (std::abs(p - pitch) < distance) {best = p; distance = std::abs(p - pitch);}
    }
    return best;
}
void sortNotes(std::vector<Note>& notes) {
    std::sort(notes.begin(), notes.end(), [](const Note& a, const Note& b) {return a.start < b.start;});
}
void fitDurations(std::vector<Note>& notes, double length) {
    sortNotes(notes);
    for (std::size_t i = 0; i < notes.size(); ++i) {
        auto& n = notes[i];
        if (n.locked) continue;
        const double next = i + 1 < notes.size() ? notes[i + 1].start : length;
        n.duration = std::max(.0011, std::min(n.duration, next - n.start));
    }
}
bool resolves(const std::vector<Note>& notes, std::size_t i, double length, const Settings& s) noexcept {
    const auto& n = notes[i];
    const auto& target = notes[(i + 1) % notes.size()];
    const int interval = std::abs(target.pitch - n.pitch);
    const double gap = target.start - n.start + (i + 1 == notes.size() ? length : 0.0);
    return gridStep(n.start) % 2 == 1 && n.duration <= .20 + epsilon
        && triadic(target.pitch, s) && interval >= 1 && interval <= 2 && gap <= .5 + epsilon;
}
void resolveConnectors(std::vector<Note>& notes, double length, const Settings& s) {
    // Targets may have moved during a response, or been replaced by a user lock.
    // An unlocked orphan is restored to triadic support instead of left hanging.
    for (std::size_t i = 0; i < notes.size(); ++i)
        if (!notes[i].locked && !triadic(notes[i].pitch, s) && !resolves(notes, i, length, s))
            notes[i].pitch = nearestTriad(notes[i].pitch, s);
}
void articulate(std::vector<Note>& notes, const Settings& s, Random& random, int accentStep) {
    const int centreVelocity = 96 + random.integer(7) - 3;
    std::vector<int> heldOptions;
    for (std::size_t i = 0; i < notes.size() && notes[i].start < 2.0; ++i) {
        double next = i + 1 < notes.size() ? notes[i + 1].start : 4.0;
        if (s.style == Style::rolling) next = std::min(next, 2.0);
        if (next - notes[i].start >= .48) heldOptions.push_back(gridStep(notes[i].start));
    }
    const int heldStep = heldOptions.empty() ? gridStep(notes.front().start)
        : heldOptions[random.integer(static_cast<int>(heldOptions.size()))];
    for (std::size_t i = 0; i < notes.size(); ++i) {
        auto& n = notes[i];
        const int step = gridStep(n.start);
        const bool anticipated = (s.style == Style::deep || s.style == Style::funk) && step == 7
            && std::none_of(notes.begin(), notes.end(), [](const Note& other) { return gridStep(other.start) == 8; });
        const bool anchor = i == 0 || anticipated || (notes[i - 1].start < 2.0 && n.start >= 2.0), beat = step % 4 == 0;
        const bool accent = s.style == Style::acid && step % 8 == accentStep;
        int velocity = centreVelocity + (anchor ? 10 : (beat ? 4 : (step % 2 ? -9 : -2)));
        if (s.style == Style::acid) velocity = centreVelocity + (accent ? 17 : -5);
        n.velocity = std::clamp(velocity + random.integer(5) - 2, 48, 123);
        double duration = .1 + .25 * s.gate;
        if (s.style == Style::deep) duration = anchor ? .35 + .85 * s.gate : .13 + .30 * s.gate;
        else if (s.style == Style::rolling) duration = step % 8 == heldStep ? .18 + .55 * s.gate : .055 + .14 * s.gate;
        else if (s.style == Style::offbeat) duration = step % 2 ? .045 + .11 * s.gate : .08 + .32 * s.gate;
        else if (s.style == Style::acid) duration = step % 8 == heldStep ? .22 + .70 * s.gate : .06 + .18 * s.gate;
        else if (s.style == Style::disco) duration = step % 2 ? .05 + .13 * s.gate
            : n.pitch == basePitch(s) + 12 ? .075 + .23 * s.gate : .12 + .37 * s.gate;
        else duration = anchor ? .13 + .30 * s.gate : .06 + .18 * s.gate;
        double next = i + 1 < notes.size() ? notes[i + 1].start : 4.0;
        if (s.style == Style::rolling && n.start < 2.0) next = std::min(next, 2.0);
        const double room = next - n.start;
        n.duration = std::max(.04, std::min(duration, room * (.74 + .20 * s.gate)));
    }
    if (s.style == Style::rolling) {
        // Two-beat rolling gesture repeats its actual accents and gates too.
        for (auto& n : notes) if (n.start >= 2.0) {
            for (const auto& first : notes) if (std::abs(first.start + 2.0 - n.start) < epsilon) {
                n.velocity = first.velocity; n.duration = first.duration; break;
            }
        }
    }
}
void addConnectors(std::vector<Note>& notes, const Settings& s, Random& random) {
    if (s.rootOnly || s.movement <= 0) return;
    const int base = basePitch(s), ceiling = std::min(base + (s.octaveJumps ? 12 : 11), 71);
    // At most two brief connectors per bar. Their next note already exists.
    int budget = static_cast<int>(std::floor(2.0 * s.movement));
    if (random.unit() < 2.0 * s.movement - budget) ++budget;
    std::array<double, 16> weights{};
    for (const auto& n : notes) {
        const int step = gridStep(n.start);
        if (step % 2 == 0) weights[step] = step == 0 || step >= 12 ? 2.0 : 1.0;
    }
    const auto targets = weightedOrder(weights, random);
    for (int targetStep : targets) {
        if (budget <= 0) break;
        const int connectorStep = (targetStep + 15) % 16;
        const double start = onset(connectorStep, s);
        if (s.anchorRoot && start < notes.front().start) continue;
        auto target = std::find_if(notes.begin(), notes.end(), [&](const Note& n) {return gridStep(n.start) == targetStep;});
        if (target == notes.end() || !triadic(target->pitch, s)) continue;
        std::vector<int> choices;
        for (int pitch = base; pitch <= ceiling; ++pitch) {
            const int distance = std::abs(pitch - target->pitch);
            if (distance >= 1 && distance <= 2 && belongsToScale(pitch, s) && !triadic(pitch, s)) choices.push_back(pitch);
        }
        if (choices.empty()) continue;
        auto existing = std::find_if(notes.begin(), notes.end(), [&](const Note& n) {return gridStep(n.start) == connectorStep;});
        if (existing == notes.end() && notes.size() >= 12) continue;
        const int pitch = choices[random.integer(static_cast<int>(choices.size()))];
        Note connector{start, .07 + .08 * s.gate, pitch, std::max(48, target->velocity - 22), false};
        if (existing != notes.end()) *existing = connector;
        else notes.push_back(connector);
        --budget;
        sortNotes(notes);
    }
    fitDurations(notes, 4.0);
    resolveConnectors(notes, 4.0, s);
}
double pitchTransition(int from, int to, const Settings& s) noexcept {
    const int distance = std::abs(to - from);
    if (distance == 0) return 1.8;
    if (distance <= 4) return 1.2;
    if (distance <= 7) return .8;
    if (distance == 12) return s.style == Style::disco ? 1.8 : .45;
    return .3;
}
int weightedState(const std::array<double, 4>& weights, Random& random) {
    double sum = 0; for (double w : weights) sum += w;
    double draw = random.unit() * sum;
    int last = 0;
    for (int i = 0; i < 4; ++i) if (weights[i] > 0) {
        last = i; if ((draw -= weights[i]) < 0) return i;
    }
    return last;
}
void composePitches(std::vector<Note>& notes, const Settings& s, Random& random) {
    const int base = basePitch(s);
    const std::array<int, 4> pitches{{base, base + third(s), base + 7, base + 12}};
    const std::array<bool, 4> active{{true, !s.rootOnly, !s.rootOnly, s.octaveJumps && base + 12 <= 71}};
    int count = static_cast<int>(notes.size());
    if (s.style == Style::rolling) {
        count = 0; for (const auto& n : notes) if (n.start < 2.0) ++count;
    }
    if (count == 0) return;
    const std::array<double, 6> colourRates{{.65, .45, .35, .65, .55, .65}};
    const double desired = s.rootOnly ? 0 : count * static_cast<double>(s.movement) * colourRates[static_cast<int>(s.style)];
    int budget = static_cast<int>(std::floor(desired));
    if (random.unit() < desired - budget) ++budget;
    budget = std::min(budget, std::max(0, count - 2));
    const auto coloured = [](int state) { return state == 1 || state == 2 ? 1 : 0; };
    int first = 0;
    if (!s.anchorRoot) first = weightedState({{1.5, active[1] && budget ? .4 : 0,
        active[2] && budget ? .6 : 0, active[3] ? .2 : 0}}, random);
    std::array<double, 4> endings{{4.0, .45 * s.movement, .8 * s.movement,
        s.style == Style::disco ? 3.0 : .45 + .55 * s.movement}};
    for (int state = 0; state < 4; ++state) {
        if (!active[state] || coloured(state) + coloured(first) > budget) endings[state] = 0;
        else endings[state] *= pitchTransition(pitches[state], pitches[first], s);
    }
    const int last = count == 1 ? first : weightedState(endings, random);
    const double crest = .3 + .4 * random.unit();
    const double peak = active[3] && random.unit() < .4 ? 12.0 : 7.0;
    const auto emission = [&](int index, int state) {
        if (!active[state] || (index == 0 && state != first) || (index == count - 1 && state != last)) return 0.0;
        const double progress = count > 1 ? static_cast<double>(index) / (count - 1) : 0;
        const double contour = peak * (progress <= crest ? progress / crest : (1 - progress) / (1 - crest));
        double weight = state == 0 ? 1.0 : state == 1 ? .9 : state == 2 ? 1.15 : .5;
        weight /= 1 + .13 * std::abs(pitches[state] - base - contour);
        const int step = gridStep(notes[index].start);
        if (s.style == Style::disco && state == 3) weight *= step % 4 >= 2 ? 12 : .4;
        else if (s.style == Style::offbeat && state == 0) weight *= 2;
        else if (s.style == Style::acid && state == 3) weight *= 1.6;
        if (step % 4 == 0 && state == 0) weight *= 1.5;
        return weight;
    };
    // Finite-state backward weighting: each forward draw includes the mass of
    // all feasible suffixes, rather than choosing a note and repairing its end.
    // The extra budget state enforces a bounded number of non-tonic attacks.
    using Row = std::array<std::array<double, 17>, 4>;
    std::vector<Row> suffix(static_cast<std::size_t>(count + 1));
    for (int previous = 0; previous < 4; ++previous) suffix[count][previous][0] = 1;
    for (int index = count - 1; index >= 0; --index)
        for (int previous = 0; previous < 4; ++previous)
            for (int remaining = 0; remaining <= budget; ++remaining)
                for (int state = 0; state < 4; ++state) {
                    const int cost = coloured(state);
                    if (cost > remaining) continue;
                    suffix[index][previous][remaining] += emission(index, state)
                        * pitchTransition(pitches[previous], pitches[state], s)
                        * suffix[index + 1][state][remaining - cost];
                }
    int previous = first, remaining = budget;
    for (int index = 0; index < count; ++index) {
        std::array<double, 4> weights{};
        for (int state = 0; state < 4; ++state) if (coloured(state) <= remaining)
            weights[state] = emission(index, state) * pitchTransition(pitches[previous], pitches[state], s)
                * suffix[index + 1][state][remaining - coloured(state)];
        const int state = weightedState(weights, random);
        notes[index].pitch = pitches[state]; remaining -= coloured(state); previous = state;
    }
    if (s.style == Style::rolling) for (auto& n : notes) if (n.start >= 2.0)
        for (int i = 0; i < count; ++i) if (std::abs(notes[i].start + 2.0 - n.start) < epsilon) {
            n.pitch = notes[i].pitch; break;
        }
}
std::vector<Note> makeMotif(const Settings& s, Random& random) {
    const auto mask = makeRhythm(s, random);
    std::vector<Note> notes;
    std::vector<int> accentOptions;
    for (int step = 0; step < 16; ++step) if (mask[step]) {
        notes.push_back({onset(step, s), .1, basePitch(s), 96, false});
        if (step > 0 && step < 8) accentOptions.push_back(step);
    }
    const int accentStep = accentOptions.empty() ? 0 : accentOptions[random.integer(static_cast<int>(accentOptions.size()))];
    composePitches(notes, s, random);
    articulate(notes, s, random, accentStep);
    addConnectors(notes, s, random);
    return notes;
}
int changedTriadPitch(int current, const Settings& s, Random& random) {
    const int base = basePitch(s);
    std::vector<int> choices;
    for (int pitch : {base, base + third(s), base + 7, base + 12}) {
        if (pitch == current || pitch > 71 || (!s.octaveJumps && pitch == base + 12)
            || (s.rootOnly && pitch % 12 != s.root)) continue;
        choices.push_back(pitch);
    }
    return choices.empty() ? current : choices[random.integer(static_cast<int>(choices.size()))];
}
std::vector<Note> response(const std::vector<Note>& motif, const Settings& s, Random& random, bool larger) {
    auto notes = motif;
    const int edits = larger && s.movement > .55 ? 2 : 1;
    std::vector<int> available;
    for (int i = 0; i < static_cast<int>(notes.size()); ++i)
        if (notes[i].start >= 2.5 && triadic(notes[i].pitch, s)) available.push_back(i);
    if (available.empty()) for (int i = 0; i < static_cast<int>(notes.size()); ++i)
        if (notes[i].start >= 2.0 && triadic(notes[i].pitch, s)) available.push_back(i);
    for (int edit = 0; edit < edits && !available.empty(); ++edit) {
        const int at = random.integer(static_cast<int>(available.size()));
        const int index = available[at];
        auto& n = notes[index]; available.erase(available.begin() + at);
        const auto& preceding = notes[(index + notes.size() - 1) % notes.size()];
        const int original = n.pitch;
        if (triadic(preceding.pitch, s) && random.unit() < .35 + .55 * s.movement)
            n.pitch = changedTriadPitch(n.pitch, s, random);
        if (n.pitch == original) {
            n.velocity = std::clamp(n.velocity + (random.unit() < .5 ? -7 : 7), 48, 123);
            n.duration *= random.unit() < .5 ? .8 : 1.1;
        }
    }
    // Occasionally displace one weak ending attack; the repeated opening stays.
    if (s.style != Style::offbeat && random.unit() < .15 + .45 * s.movement) {
        for (auto it = notes.rbegin(); it != notes.rend(); ++it) {
            int step = gridStep(it->start);
            if (step < 12 || step % 4 == 0 || !triadic(it->pitch, s)) continue;
            if (s.style == Style::disco && step % 2 == 0) continue;
            const int destination = step == 15 ? 14 : step + 1;
            if (std::any_of(notes.begin(), notes.end(), [&](const Note& n) {return gridStep(n.start) == destination;})) continue;
            it->start = onset(destination, s); break;
        }
    }
    fitDurations(notes, 4.0);
    resolveConnectors(notes, 4.0, s);
    return notes;
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
    std::vector<Note> locked;
    if (previous != nullptr) for (int i = 0; i < previous->count; ++i) {
        const auto& n = previous->notes[i];
        if (n.locked && n.start + n.duration <= result.lengthBeats + epsilon) locked.push_back(n);
    }
    // A compatible Variation edits the existing phrase. It never overlays a
    // fresh rhythm, so repeated clicks cannot progressively fill every rest.
    bool compatible = variation && previous != nullptr && previous->count > 0
        && previous->lengthBeats == result.lengthBeats;
    std::vector<Note> prior;
    if (compatible) {
        prior.assign(previous->notes.begin(), previous->notes.begin() + previous->count);
        const int base = basePitch(s), ceiling = std::min(base + (s.octaveJumps ? 12 : 11), 71);
        for (std::size_t i = 0; i < prior.size(); ++i) {
            const auto& n = prior[i];
            if (n.locked) continue;
            if (n.pitch < base || n.pitch > ceiling || !belongsToScale(n.pitch, s)
                || (s.rootOnly && n.pitch % 12 != s.root)
                || std::abs(n.start - onset(gridStep(n.start), s)) > 1.e-6
                || (!triadic(n.pitch, s) && !resolves(prior, i, result.lengthBeats, s))) compatible = false;
        }
        if (!prior.empty() && s.anchorRoot && !prior.front().locked && prior.front().pitch != base) compatible = false;
    }
    if (compatible) {
        std::vector<int> editable;
        for (int i = 0; i < static_cast<int>(prior.size()); ++i)
            if (!prior[i].locked && std::fmod(prior[i].start, 4.0) >= 2.0 && triadic(prior[i].pitch, s)) editable.push_back(i);
        const int budget = std::max(1, static_cast<int>(std::ceil(prior.size() * .12)));
        for (int edit = 0; edit < budget && !editable.empty(); ++edit) {
            const int slot = random.integer(static_cast<int>(editable.size()));
            const int index = editable[slot]; editable.erase(editable.begin() + slot);
            auto& n = prior[index];
            const auto& preceding = prior[(index + prior.size() - 1) % prior.size()];
            const int oldPitch = n.pitch;
            // Do not move the target of a prepared connector.
            if (triadic(preceding.pitch, s) && random.unit() < .35 + .55 * s.movement)
                n.pitch = changedTriadPitch(n.pitch, s, random);
            if (n.pitch == oldPitch) {
                n.velocity = std::clamp(n.velocity + (random.unit() < .5 ? -6 : 6), 48, 123);
                const double next = index + 1 < static_cast<int>(prior.size()) ? prior[index + 1].start : result.lengthBeats;
                // A preserved off-grid lock may leave less than the usual
                // articulation floor. Never enlarge beyond that available gap.
                n.duration = std::min(std::max(.0011, n.duration * (random.unit() < .5 ? .85 : 1.1)),
                    std::max(n.duration, (next - n.start) * .94));
            }
        }
        for (const auto& n : prior) result.notes[result.count++] = n;
        return result;
    }
    const auto motif = makeMotif(s, random);
    const auto answerB = response(motif, s, random, false);
    const auto answerC = response(motif, s, random, true);
    const bool alternating = random.unit() < .5;
    std::vector<Note> candidates;
    for (int bar = 0; bar < s.bars; ++bar) {
        const std::vector<Note>* selected = &motif;
        if (bar % 4 == 3) selected = &answerC;
        else if (bar % 2 == 1 && (alternating || s.bars == 2)) selected = &answerB;
        for (auto n : *selected) {n.start += bar * 4.0; candidates.push_back(n);}
    }
    // Locks are exact user decisions. They outrank new notes, and are reserved
    // before filling the remaining capacity, even when they lie near loop end.
    std::vector<Note> assembled = locked;
    assembled.reserve(maxNotes);
    for (auto n : candidates) {
        if (assembled.size() >= maxNotes) break;
        n.duration = std::min(n.duration, result.lengthBeats - n.start);
        bool reject = false;
        for (const auto& kept : locked) {
            if (n.start >= kept.start - epsilon && n.start < kept.start + kept.duration - epsilon) {reject = true; break;}
            if (intersects(n, kept)) n.duration = std::min(n.duration, kept.start - n.start);
        }
        if (!reject && n.duration >= .0011) assembled.push_back(n);
    }
    fitDurations(assembled, result.lengthBeats);
    if (!assembled.empty() && s.anchorRoot && !assembled.front().locked) assembled.front().pitch = basePitch(s);
    resolveConnectors(assembled, result.lengthBeats, s);
    for (const auto& n : assembled) result.notes[result.count++] = n;
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
