
#include <iostream>
#include <string>
#include <vector>
#include <array>
#include <cctype>
#include <algorithm>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include "hand_analyzer.hpp"

enum class Suit {
    BAMBOO,
    DOT,
    CHARACTER,
    WINDS,
    DRAGONS,
    FLOWER,
    JOKER
};

struct Tile {
    Suit suit;
    int rank;

    constexpr Tile(Suit s, int r) : suit(s), rank(r) {}
};

constexpr char suitCharConv(Suit s) {
    switch (s) {
        case Suit::BAMBOO:    return 'B';
        case Suit::DOT:       return 'D';
        case Suit::CHARACTER: return 'C';
        case Suit::WINDS:     return 'W';
        case Suit::DRAGONS:   return 'Z';
        case Suit::FLOWER:    return 'F';
        case Suit::JOKER:     return 'J';
    }
    return '?';
}

std::string tileToString(const Tile& tile) {
    if (tile.suit == Suit::WINDS) {
        return std::string(1, "ESWN"[tile.rank - 1]);
    }

    if (tile.suit == Suit::DRAGONS) {
        if (tile.rank == 1) return "RD";
        if (tile.rank == 2) return "GD";
        if (tile.rank == 3) return "WD";
    }

    if (tile.suit == Suit::FLOWER) return "F";
    if (tile.suit == Suit::JOKER) return "J";

    return std::to_string(tile.rank) + suitCharConv(tile.suit);
}

bool parseTile(const std::string& input, Suit& outSuit, int& outRank) {
    if (input.size() == 1) {
        char c = std::toupper(static_cast<unsigned char>(input[0]));

        switch (c) {
            case 'E': outSuit = Suit::WINDS; outRank = 1; return true;
            case 'S': outSuit = Suit::WINDS; outRank = 2; return true;
            case 'W': outSuit = Suit::WINDS; outRank = 3; return true;
            case 'N': outSuit = Suit::WINDS; outRank = 4; return true;
            case 'F': outSuit = Suit::FLOWER; outRank = 0; return true;
            case 'J': outSuit = Suit::JOKER; outRank = 0; return true;
            default: return false;
        }
    }

    if (input.size() != 2) return false;

    char rankChar = std::toupper(static_cast<unsigned char>(input[0]));
    char suitChar = std::toupper(static_cast<unsigned char>(input[1]));

    if (suitChar == 'D') {
        if (rankChar == 'R') {
            outSuit = Suit::DRAGONS;
            outRank = 1;
            return true;
        }
        if (rankChar == 'G') {
            outSuit = Suit::DRAGONS;
            outRank = 2;
            return true;
        }
        if (rankChar == 'W') {
            outSuit = Suit::DRAGONS;
            outRank = 3;
            return true;
        }
    }

    if (rankChar < '1' || rankChar > '9') return false;

    Suit suit;

    if (suitChar == 'B') suit = Suit::BAMBOO;
    else if (suitChar == 'D') suit = Suit::DOT;
    else if (suitChar == 'C') suit = Suit::CHARACTER;
    else return false;

    outRank = rankChar - '0';
    outSuit = suit;

    return true;
}

bool sameTile(const Tile& a, const Tile& b) {
    return a.suit == b.suit && a.rank == b.rank;
}

int countCopies(const std::vector<Tile>& hand, const Tile& target) {
    int count = 0;

    for (const Tile& tile : hand) {
        if (sameTile(tile, target)) {
            count++;
        }
    }

    return count;
}

int maxCopies(const Tile& tile) {
    if (tile.suit == Suit::FLOWER ||
        tile.suit == Suit::JOKER) {
        return 8;
    }

    return 4;
}

// NEW: Map each tile to a unique index from 0 to 35
constexpr int tileIndex(const Tile& tile) {
    switch (tile.suit) {
        case Suit::BAMBOO:
            return tile.rank - 1;          // 0-8

        case Suit::DOT:
            return 9 + tile.rank - 1;      // 9-17

        case Suit::CHARACTER:
            return 18 + tile.rank - 1;     // 18-26

        case Suit::WINDS:
            return 27 + tile.rank - 1;     // 27-30

        case Suit::DRAGONS:
            return 31 + tile.rank - 1;     // 31-33

        case Suit::FLOWER:
            return 34;

        case Suit::JOKER:
            return 35;
    }

    return -1;
}

Tile tileFromIndex(int index) {
    if (index < 0 || index >= mahjong::TILE_TYPES) {
        throw std::out_of_range("Invalid tile index");
    }
    if (index < 9) return {Suit::BAMBOO, index + 1};
    if (index < 18) return {Suit::DOT, index - 9 + 1};
    if (index < 27) return {Suit::CHARACTER, index - 18 + 1};
    if (index < 31) return {Suit::WINDS, index - 27 + 1};
    if (index < 34) return {Suit::DRAGONS, index - 31 + 1};
    if (index == 34) return {Suit::FLOWER, 0};
    return {Suit::JOKER, 0};
}

std::vector<Tile> parseTileList(std::string text) {
    std::replace(text.begin(), text.end(), ',', ' ');
    std::istringstream stream(text);
    std::vector<Tile> tiles;
    std::string token;
    while (stream >> token) {
        Suit suit;
        int rank;
        if (!parseTile(token, suit, rank)) {
            throw std::invalid_argument("Invalid tile: " + token);
        }
        tiles.emplace_back(suit, rank);
    }
    return tiles;
}

struct Options {
    std::string patternFile = "demo_patterns.txt";
    std::string handText;
    std::string visibleText;
    bool handProvided = false;
    bool showAll = false;
};

void printUsage() {
    std::cout << "Usage: ./mahjong [--patterns FILE] [--hand TILES] [--visible TILES] [--all]\n"
              << "\n"
              << "  --patterns FILE   14-tile patterns (default: demo_patterns.txt)\n"
              << "  --hand TILES      Exactly 14 tiles; otherwise prompted interactively\n"
              << "  --visible TILES   Exposed/discarded tiles (not including your hand)\n"
              << "  --all             Show all discard choices instead of the top five\n"
              << "  --help            Show this help message\n"
              << "\n"
              << "Tile examples: 5B, 6D, 9C, E, S, W, N, RD, GD, WD, F, J\n"
              << "Separate tile codes with spaces or commas; quote each tile list.\n";
}

Options parseOptions(int argc, char* argv[]) {
    Options options;
    for (int i = 1; i < argc; ++i) {
        const std::string option = argv[i];
        if (option == "--all") {
            options.showAll = true;
        } else if (option == "--patterns" || option == "--hand" || option == "--visible") {
            if (i + 1 >= argc) {
                throw std::invalid_argument("Missing value for " + option);
            }
            const std::string value = argv[++i];
            if (option == "--patterns") options.patternFile = value;
            else if (option == "--hand") {
                options.handText = value;
                options.handProvided = true;
            } else options.visibleText = value;
        } else {
            throw std::invalid_argument("Unknown option: " + option + " (try --help)");
        }
    }
    return options;
}

int main(int argc, char* argv[]) {
    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "--help") {
            printUsage();
            return 0;
        }
    }

    try {
        const Options options = parseOptions(argc, argv);
        const auto patterns = mahjong::loadPatterns(
            options.patternFile,
            [](const std::string& label) {
                Suit suit = Suit::BAMBOO;
                int rank = 0;
                if (!parseTile(label, suit, rank)) return -1;
                return tileIndex(Tile(suit, rank));
            });

        mahjong::Counts visible{};
        for (const Tile& tile : parseTileList(options.visibleText)) {
            const int index = tileIndex(tile);
            if (visible[index] >= maxCopies(tile)) {
                throw std::invalid_argument("Too many visible copies of " + tileToString(tile));
            }
            ++visible[index];
        }

        std::vector<Tile> hand;
        hand.reserve(14);
        mahjong::Counts counts{};

        if (options.handProvided) {
            hand = parseTileList(options.handText);
            if (hand.size() != 14) {
                throw std::invalid_argument("--hand requires exactly 14 tiles");
            }
            for (const Tile& tile : hand) {
                const int index = tileIndex(tile);
                if (++counts[index] + visible[index] > maxCopies(tile)) {
                    throw std::invalid_argument("Too many hand/visible copies of " + tileToString(tile));
                }
            }
        } else {
            std::cout << "Welcome to Mahjong Analyzer!\nEnter 14 tiles:\n";
            while (hand.size() < 14) {
                std::string input;
                if (!(std::cin >> input)) {
                    std::cerr << "\nInput stream closed. Exiting.\n";
                    return 1;
                }
                Suit suit;
                int rank;
                if (!parseTile(input, suit, rank)) {
                    std::cout << "Invalid tile. Retry.\n";
                    continue;
                }
                Tile tile(suit, rank);
                const int index = tileIndex(tile);
                if (counts[index] + visible[index] >= maxCopies(tile)) {
                    std::cout << "Too many copies of this tile (including visible tiles)!\n";
                    continue;
                }
                hand.push_back(tile);
                ++counts[index];
            }
        }

        const int unknownCopies = mahjong::unseenCopies(counts, visible);
        std::cout << "\nYour hand: ";
        for (const Tile& tile : hand) std::cout << tileToString(tile) << " ";

        std::cout << "\n\nTile frequencies:\n";
        std::array<bool, mahjong::TILE_TYPES> printed{};
        for (const Tile& tile : hand) {
            const int index = tileIndex(tile);
            if (printed[index]) continue;
            std::cout << tileToString(tile) << ": " << counts[index]
                      << " / " << maxCopies(tile) << "\n";
            printed[index] = true;
        }

        const auto choices = mahjong::rankDiscards(counts, patterns, visible);
        std::cout << "\nDiscard rankings (patterns: " << options.patternFile << "):\n"
                  << "Uniform-unknown-draw model: " << unknownCopies
                  << " unseen copies; does not model opponent hands or actual wall contents.\n";

        const std::size_t limit = options.showAll ? choices.size()
            : std::min<std::size_t>(5, choices.size());
        for (std::size_t i = 0; i < limit; ++i) {
            const auto& choice = choices[i];
            const double percent = unknownCopies == 0 ? 0.0
                : 100.0 * choice.improvingCopies / unknownCopies;

            std::cout << (i + 1) << ". Discard " << tileToString(tileFromIndex(choice.tileIndex))
                      << " | missing: " << choice.missing
                      << " | improving copies: " << choice.improvingCopies
                      << " | improving types: " << choice.improvingTileTypes
                      << " | estimated improvement: " << std::fixed << std::setprecision(1)
                      << percent << "%"
                      << " | nearest pattern: " << choice.closestPattern << "\n";
            std::cout << "   Improving draws: ";
            if (choice.improvingDraws.empty()) {
                std::cout << "none";
            } else {
                for (std::size_t j = 0; j < choice.improvingDraws.size(); ++j) {
                    const auto& draw = choice.improvingDraws[j];
                    if (j != 0) std::cout << ", ";
                    std::cout << tileToString(tileFromIndex(draw.tileIndex))
                              << " (" << draw.remainingCopies << " left)";
                }
            }
            std::cout << "\n   To finish nearest pattern: ";
            if (choice.remainingRequirements.empty()) {
                std::cout << "nothing missing";
            } else {
                bool first = true;
                for (const auto& requirement : choice.remainingRequirements) {
                    const std::string tile = tileToString(tileFromIndex(requirement.tileIndex));
                    if (requirement.naturalOnly > 0) {
                        if (!first) std::cout << ", ";
                        std::cout << requirement.naturalOnly << "x" << tile
                                  << " (natural only)";
                        first = false;
                    }
                    if (requirement.jokerEligible > 0) {
                        if (!first) std::cout << ", ";
                        std::cout << requirement.jokerEligible << "x" << tile
                                  << " (or J)";
                        first = false;
                    }
                }
            }
            std::cout << "\n";
        }
    } catch (const std::exception& error) {
        std::cerr << "Error: " << error.what() << "\n";
        return 1;
    }
    return 0;
}
