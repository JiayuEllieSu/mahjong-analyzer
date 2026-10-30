#pragma once

// American Mahjong analysis against explicit 14-tile card patterns.
// The engine uses tile counts, so main.cpp can keep its existing Tile struct.

#include <algorithm>
#include <array>
#include <charconv>
#include <fstream>
#include <functional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <system_error>
#include <utility>
#include <vector>

namespace mahjong {

constexpr int TILE_TYPES = 36;
constexpr int JOKER_INDEX = 35;
using Counts = std::array<int, TILE_TYPES>;

struct Group {
    int tileIndex;  // 0-34 (non-joker tile)
    int size;       // 1 = single, 2 = pair, 3+ = joker-eligible set
};

struct Pattern {
    std::string name;
    std::vector<Group> groups;
};

struct Match {
    int missing;        // Tiles needed to finish a 14-tile pattern
    int usefulNaturals; // Natural tiles already matching the pattern
    int usefulJokers;   // Jokers that can legally fill its groups
};

struct DiscardChoice {
    int tileIndex;
    int missing;
    int improvingCopies;
    int improvingTileTypes;
    std::string closestPattern;
};

inline int physicalCopies(int tileIndex) {
    return (tileIndex == 34 || tileIndex == JOKER_INDEX) ? 8 : 4;
}

inline std::string trim(const std::string& text) {
    const auto first = text.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return "";
    const auto last = text.find_last_not_of(" \t\r\n");
    return text.substr(first, last - first + 1);
}

// One line per concrete pattern:
//   My example | 2*F 3*5B 3*6B 3*7B 3*8B
// The caller supplies its existing tile-string parser as 'indexOf'.
inline std::vector<Pattern> loadPatterns(
    const std::string& filename,
    const std::function<int(const std::string&)>& indexOf) {

    std::ifstream file(filename);
    if (!file) throw std::runtime_error("Cannot open pattern file: " + filename);

    std::vector<Pattern> patterns;
    std::string line;
    int lineNumber = 0;

    while (std::getline(file, line)) {
        ++lineNumber;
        line = trim(line.substr(0, line.find('#')));
        if (line.empty()) continue;

        const auto separator = line.find('|');
        if (separator == std::string::npos) {
            throw std::runtime_error("Expected '|' on line " + std::to_string(lineNumber));
        }

        Pattern pattern;
        pattern.name = trim(line.substr(0, separator));
        if (pattern.name.empty()) {
            throw std::runtime_error("Missing pattern name on line " + std::to_string(lineNumber));
        }

        std::istringstream groups(line.substr(separator + 1));
        std::string token;
        int total = 0;

        while (groups >> token) {
            const auto star = token.find('*');
            if (star == std::string::npos) {
                throw std::runtime_error("Expected size*tile on line " + std::to_string(lineNumber));
            }

            int size = 0;
            const auto number = token.substr(0, star);
            const auto result = std::from_chars(number.data(), number.data() + number.size(), size);
            if (result.ec != std::errc{} || result.ptr != number.data() + number.size() || size < 1 || size > 8) {
                throw std::runtime_error("Invalid group size on line " + std::to_string(lineNumber));
            }

            const int index = indexOf(token.substr(star + 1));
            if (index < 0 || index >= JOKER_INDEX) {
                throw std::runtime_error("Invalid tile in pattern on line " + std::to_string(lineNumber));
            }

            pattern.groups.push_back({index, size});
            total += size;
        }

        if (total != 14) {
            throw std::runtime_error("Pattern must have 14 tiles on line " + std::to_string(lineNumber));
        }
        patterns.push_back(std::move(pattern));
    }

    if (patterns.empty()) throw std::runtime_error("No patterns found in " + filename);
    return patterns;
}

// Find the maximum number of tiles we can retain toward one card pattern.
// Singles and pairs MUST be filled with real tiles; jokers can fill any
// number of positions in groups of 3+ identical tiles (including flowers).
inline Match matchPattern(const Counts& hand, const Pattern& pattern) {
    std::array<int, JOKER_INDEX> fixed{};    // Singles and pairs: no jokers
    std::array<int, JOKER_INDEX> flexible{}; // Pungs, kongs, quints, etc.

    for (const Group& group : pattern.groups) {
        if (group.size <= 2) fixed[group.tileIndex] += group.size;
        else flexible[group.tileIndex] += group.size;
    }

    int naturalMatches = 0;
    int jokerSlots = 0;

    for (int tile = 0; tile < JOKER_INDEX; ++tile) {
        // Reserve real tiles for singles/pairs before using them in larger sets.
        const int fixedMatches = std::min(hand[tile], fixed[tile]);
        const int leftoverReal = hand[tile] - fixedMatches;
        const int flexibleMatches = std::min(leftoverReal, flexible[tile]);

        naturalMatches += fixedMatches + flexibleMatches;
        jokerSlots += flexible[tile] - flexibleMatches;
    }

    const int usefulJokers = std::min(hand[JOKER_INDEX], jokerSlots);
    return {14 - naturalMatches - usefulJokers, naturalMatches, usefulJokers};
}

inline int minimumMissing(const Counts& hand, const std::vector<Pattern>& patterns) {
    int best = 14;
    for (const Pattern& pattern : patterns) {
        best = std::min(best, matchPattern(hand, pattern).missing);
    }
    return best;
}

// Uses a uniform-unknown-tile model, NOT opponent strategy or wall composition.
// 'visible' contains already exposed/discarded tiles, excluding your 14 tiles.
inline std::vector<DiscardChoice> rankDiscards(
    const Counts& hand, const std::vector<Pattern>& patterns,
    const Counts& visible = Counts{}) {

    if (patterns.empty()) throw std::invalid_argument("Provide at least one pattern");

    int handSize = 0;
    for (int tile = 0; tile < TILE_TYPES; ++tile) {
        if (hand[tile] < 0 || visible[tile] < 0 ||
            hand[tile] + visible[tile] > physicalCopies(tile)) {
            throw std::invalid_argument("Impossible hand or visible tile counts");
        }
        handSize += hand[tile];
    }
    if (handSize != 14) throw std::invalid_argument("Expected a 14-tile hand");

    std::vector<DiscardChoice> choices;
    Counts afterDiscard = hand;

    for (int discard = 0; discard < TILE_TYPES; ++discard) {
        if (hand[discard] == 0) continue;
        --afterDiscard[discard];

        int bestMissing = 15;
        std::string bestName;
        for (const Pattern& pattern : patterns) {
            const int missing = matchPattern(afterDiscard, pattern).missing;
            if (missing < bestMissing) {
                bestMissing = missing;
                bestName = pattern.name;
            }
        }

        int improvingCopies = 0;
        int improvingTypes = 0;
        for (int draw = 0; draw < TILE_TYPES; ++draw) {
            // Use the ORIGINAL 14-tile hand: the discarded tile is now known,
            // not a tile that can still be drawn from the unknown pool.
            const int unseen = physicalCopies(draw) - hand[draw] - visible[draw];
            if (unseen == 0) continue;

            ++afterDiscard[draw];
            if (minimumMissing(afterDiscard, patterns) < bestMissing) {
                improvingCopies += unseen;
                ++improvingTypes;
            }
            --afterDiscard[draw];
        }

        choices.push_back({discard, bestMissing, improvingCopies, improvingTypes, bestName});
        ++afterDiscard[discard];
    }

    std::sort(choices.begin(), choices.end(), [](const DiscardChoice& a, const DiscardChoice& b) {
        if (a.missing != b.missing) return a.missing < b.missing;
        if (a.improvingCopies != b.improvingCopies) return a.improvingCopies > b.improvingCopies;
        return a.tileIndex < b.tileIndex;
    });
    return choices;
}

} // namespace mahjong
