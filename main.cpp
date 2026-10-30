
#include <iostream>
#include <string>
#include <vector>
#include <array>
#include <cctype>

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

int main() {
    std::vector<Tile> hand;
    hand.reserve(14);

    // NEW: 36 entries initialized to zero
    std::array<int, 36> counts{};

    std::cout << "Welcome to Mahjong Analyzer!\n";
    std::cout << "Enter 14 tiles:\n";

    while (hand.size() < 14) {
        std::string input;

        if (!(std::cin >> input)) {
            std::cout << "\nInput stream closed. Exiting.\n";
            return 1;
        }

        Suit parsedSuit;
        int parsedRank;

        if (!parseTile(input, parsedSuit, parsedRank)) {
            std::cout << "Invalid tile. Retry.\n";
            continue;
        }

        Tile candidate(parsedSuit, parsedRank);

        // Check duplicates using the array
        int index = tileIndex(candidate);

        if (counts[index] >= maxCopies(candidate)) {
            std::cout << "Too many copies of this tile!\n";
            continue;
        }

        hand.push_back(candidate);

        // Update the corresponding tile count
        counts[index]++;
    }

    std::cout << "\nYour hand: ";

    for (int i = 0; i < 14; i++) {
        std::cout << tileToString(hand[i]) << " ";
    }

    std::cout << "\n\nTile frequencies:\n";

    // Print the count of each distinct tile
    std::array<bool, 36> printed{};

    for (const Tile& tile : hand) {
        int index = tileIndex(tile);

        if (printed[index]) continue;

        std::cout << tileToString(tile)
                  << ": " << counts[index]
                  << " / " << maxCopies(tile) << "\n";

        printed[index] = true;
    }

    return 0;
}
