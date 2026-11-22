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

struct ImprovingDraw {
    int tileIndex;
    int remainingCopies; // Unseen copies in the uniform-unknown-tile model
    int missingAfterDraw;
};

// Outstanding tiles for one concrete pattern. Joker-eligible slots may be
// completed by either the named natural tile or a joker.
struct TileRequirement {
    int tileIndex;
    int naturalOnly;
    int jokerEligible;
};

struct DiscardChoice {
    int tileIndex;
    int missing;
    int improvingCopies;
    int improvingTileTypes;
    std::string closestPattern;
    std::vector<ImprovingDraw> improvingDraws;
    std::vector<TileRequirement> remainingRequirements;
};

// Advice for the 13-tile hand between turns. Improving draws reduce the
// minimum missing count against at least one still-reachable card pattern.
struct DrawAdvice {
    int missing;
    int improvingCopies;
    int improvingTileTypes;
    std::string closestPattern;
    std::vector<ImprovingDraw> improvingDraws;
    std::vector<TileRequirement> remainingRequirements;
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

// Describe one consistent way to complete a target pattern. Allocate natural
// tiles to singles/pairs first, then to sets, then use held jokers for open
// set positions in ascending tile-index order. Other joker allocations may
// be equally valid; this is an explanation, not a unique completion plan.
inline std::vector<TileRequirement> missingRequirements(
    const Counts& hand, const Pattern& pattern) {
    std::array<int, JOKER_INDEX> fixed{};
    std::array<int, JOKER_INDEX> flexible{};
    for (const Group& group : pattern.groups) {
        if (group.size <= 2) fixed[group.tileIndex] += group.size;
        else flexible[group.tileIndex] += group.size;
    }

    int jokersLeft = hand[JOKER_INDEX];
    std::vector<TileRequirement> result;
    for (int tile = 0; tile < JOKER_INDEX; ++tile) {
        const int fixedMatches = std::min(hand[tile], fixed[tile]);
        const int naturalOnly = fixed[tile] - fixedMatches;
        const int flexMatches = std::min(hand[tile] - fixedMatches, flexible[tile]);
        const int flexDeficit = flexible[tile] - flexMatches;
        const int allocatedJokers = std::min(jokersLeft, flexDeficit);
        jokersLeft -= allocatedJokers;
        const int jokerEligible = flexDeficit - allocatedJokers;
        if (naturalOnly > 0 || jokerEligible > 0) {
            result.push_back({tile, naturalOnly, jokerEligible});
        }
    }
    return result;
}

inline int minimumMissing(const Counts& hand, const std::vector<Pattern>& patterns) {
    int best = 14;
    for (const Pattern& pattern : patterns) {
        best = std::min(best, matchPattern(hand, pattern).missing);
    }
    return best;
}

// Validates physical tile availability and returns the number of tiles not in
// the hand or visible pool. These are not necessarily still in the wall.
inline int unseenCopies(const Counts& hand, const Counts& visible = Counts{}) {
    int handSize = 0;
    int unseen = 0;
    for (int tile = 0; tile < TILE_TYPES; ++tile) {
        if (hand[tile] < 0 || visible[tile] < 0 ||
            hand[tile] + visible[tile] > physicalCopies(tile)) {
            throw std::invalid_argument("Impossible hand or visible tile counts");
        }
        handSize += hand[tile];
        unseen += physicalCopies(tile) - hand[tile] - visible[tile];
    }
    if (handSize != 13 && handSize != 14) {
        throw std::invalid_argument("Expected a 13- or 14-tile hand");
    }
    return unseen;
}

// A pattern is reachable only if its required natural singles/pairs are
// still available and the remaining joker supply can cover shortages in sets.
// 'unavailable' means exposed/previously discarded tiles PLUS the current
// candidate discard. Hidden opponent tiles are not known, so this is a
// necessary feasibility check, not a guarantee of eventual completion.
inline bool canCompletePattern(const Pattern& pattern, const Counts& unavailable) {
    std::array<int, JOKER_INDEX> fixed{};
    std::array<int, JOKER_INDEX> flexible{};
    for (const Group& group : pattern.groups) {
        if (group.size <= 2) fixed[group.tileIndex] += group.size;
        else flexible[group.tileIndex] += group.size;
    }

    const int jokersAvailable = physicalCopies(JOKER_INDEX) - unavailable[JOKER_INDEX];
    if (jokersAvailable < 0 || unavailable[JOKER_INDEX] < 0) return false;

    int minimumJokers = 0;
    for (int tile = 0; tile < JOKER_INDEX; ++tile) {
        const int naturalsAvailable = physicalCopies(tile) - unavailable[tile];
        if (unavailable[tile] < 0 || naturalsAvailable < fixed[tile]) {
            return false; // Jokers cannot replace singles or pairs.
        }
        minimumJokers += std::max(0, fixed[tile] + flexible[tile] - naturalsAvailable);
        if (minimumJokers > jokersAvailable) return false;
    }
    return true;
}

// Uses a uniform-unknown-tile model, NOT opponent strategy or wall composition.
// 'visible' contains already exposed/discarded tiles, excluding your 14 tiles.
// Improving draws are measured relative to the 13-tile hand after discarding.
inline std::vector<DiscardChoice> rankDiscards(
    const Counts& hand, const std::vector<Pattern>& patterns,
    const Counts& visible = Counts{}) {

    if (patterns.empty()) throw std::invalid_argument("Provide at least one pattern");

    (void)unseenCopies(hand, visible);
    int handSize = 0;
    for (int count : hand) handSize += count;
    if (handSize != 14) {
        throw std::invalid_argument("Discard rankings require a 14-tile hand");
    }

    std::vector<DiscardChoice> choices;
    Counts afterDiscard = hand;

    for (int discard = 0; discard < TILE_TYPES; ++discard) {
        if (hand[discard] == 0) continue;
        --afterDiscard[discard];

        // The just-discarded tile is public and cannot be drawn back.
        // Filter out targets that no remaining supply could ever finish.
        Counts unavailable = visible;
        ++unavailable[discard];
        std::vector<Pattern> feasiblePatterns;
        for (const Pattern& pattern : patterns) {
            if (canCompletePattern(pattern, unavailable)) {
                feasiblePatterns.push_back(pattern);
            }
        }
        if (feasiblePatterns.empty()) {
            choices.push_back({discard, 15, 0, 0, "none (unreachable)", {}, {}});
            ++afterDiscard[discard];
            continue;
        }

        int bestMissing = 15;
        std::string bestName;
        std::size_t bestPatternIndex = 0;
        for (std::size_t index = 0; index < feasiblePatterns.size(); ++index) {
            const Pattern& pattern = feasiblePatterns[index];
            const int missing = matchPattern(afterDiscard, pattern).missing;
            if (missing < bestMissing) {
                bestMissing = missing;
                bestName = pattern.name;
                bestPatternIndex = index;
            }
        }

        int improvingCopies = 0;
        int improvingTypes = 0;
        std::vector<ImprovingDraw> improvingDraws;
        for (int draw = 0; draw < TILE_TYPES; ++draw) {
            // Use the ORIGINAL 14-tile hand: the discarded tile is now known,
            // not a tile that can still be drawn from the unknown pool.
            const int unseen = physicalCopies(draw) - hand[draw] - visible[draw];
            if (unseen == 0) continue;

            ++afterDiscard[draw];
            const int missingAfterDraw = minimumMissing(afterDiscard, feasiblePatterns);
            --afterDiscard[draw];
            if (missingAfterDraw < bestMissing) {
                improvingCopies += unseen;
                ++improvingTypes;
                improvingDraws.push_back({draw, unseen, missingAfterDraw});
            }
        }
        std::sort(improvingDraws.begin(), improvingDraws.end(),
                  [](const ImprovingDraw& a, const ImprovingDraw& b) {
            if (a.missingAfterDraw != b.missingAfterDraw) {
                return a.missingAfterDraw < b.missingAfterDraw;
            }
            if (a.remainingCopies != b.remainingCopies) {
                return a.remainingCopies > b.remainingCopies;
            }
            return a.tileIndex < b.tileIndex;
        });

        choices.push_back({discard, bestMissing, improvingCopies,
                           improvingTypes, bestName, std::move(improvingDraws),
                           missingRequirements(afterDiscard, feasiblePatterns[bestPatternIndex])});
        ++afterDiscard[discard];
    }

    std::sort(choices.begin(), choices.end(), [](const DiscardChoice& a, const DiscardChoice& b) {
        if (a.missing != b.missing) return a.missing < b.missing;
        if (a.improvingCopies != b.improvingCopies) return a.improvingCopies > b.improvingCopies;
        return a.tileIndex < b.tileIndex;
    });
    return choices;
}


 // Analyze a 13-tile hand before the next draw. Only tiles not held or
 // reported visible count as potential draws. The resulting percentage is a
 // uniform-unknown-pool estimate, NOT the probability of drawing from the wall.
inline DrawAdvice rankDraws(
    const Counts& hand, const std::vector<Pattern>& patterns,
    const Counts& visible = Counts{}) {

    if (patterns.empty()) throw std::invalid_argument("Provide at least one pattern");
    (void)unseenCopies(hand, visible);

    int handSize = 0;
    for (int count : hand) handSize += count;
    if (handSize != 13) {
        throw std::invalid_argument("Draw advice requires a 13-tile hand");
    }

    std::vector<Pattern> feasiblePatterns;
    for (const Pattern& pattern : patterns) {
        if (canCompletePattern(pattern, visible)) feasiblePatterns.push_back(pattern);
    }
    if (feasiblePatterns.empty()) {
        return {15, 0, 0, "none (unreachable)", {}, {}};
    }

    int bestMissing = 15;
    std::size_t bestPatternIndex = 0;
    for (std::size_t i = 0; i < feasiblePatterns.size(); ++i) {
        const int missing = matchPattern(hand, feasiblePatterns[i]).missing;
        if (missing < bestMissing) {
            bestMissing = missing;
            bestPatternIndex = i;
        }
    }

    Counts withDraw = hand;
    int improvingCopies = 0;
    int improvingTypes = 0;
    std::vector<ImprovingDraw> improvingDraws;
    for (int draw = 0; draw < TILE_TYPES; ++draw) {
        const int unseen = physicalCopies(draw) - hand[draw] - visible[draw];
        if (unseen <= 0) continue;
        ++withDraw[draw];
        const int missingAfterDraw = minimumMissing(withDraw, feasiblePatterns);
        --withDraw[draw];
        if (missingAfterDraw < bestMissing) {
            improvingCopies += unseen;
            ++improvingTypes;
            improvingDraws.push_back({draw, unseen, missingAfterDraw});
        }
    }

    std::sort(improvingDraws.begin(), improvingDraws.end(),
              [](const ImprovingDraw& a, const ImprovingDraw& b) {
        if (a.missingAfterDraw != b.missingAfterDraw) {
            return a.missingAfterDraw < b.missingAfterDraw;
        }
        if (a.remainingCopies != b.remainingCopies) {
            return a.remainingCopies > b.remainingCopies;
        }
        return a.tileIndex < b.tileIndex;
    });

    return {bestMissing, improvingCopies, improvingTypes,
            feasiblePatterns[bestPatternIndex].name,
            std::move(improvingDraws),
            missingRequirements(hand, feasiblePatterns[bestPatternIndex])};
}

} // namespace mahjong
