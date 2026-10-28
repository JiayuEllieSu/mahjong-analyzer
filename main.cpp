#include <iostream>
#include <string>
#include <vector>

enum class Suit {
    BAMBOO,
    DOT,
    CHARACTER,
    WINDS,
    DRAGONS
};

struct Tile {
    Suit suit;
    int rank;

    constexpr Tile(Suit s, int r) : suit(s), rank(r) {}
};

constexpr char suitCharConv(Suit s) {
    switch (s) {
        case Suit::BAMBOO:   return 'B';
        case Suit::DOT:      return 'D';
        case Suit::CHARACTER:return 'C';
        case Suit::WINDS: return 'W';
        case Suit::DRAGONS: return 'Z';
    }
    return '?';
}

bool parseTile(const std::string& input, Suit& outSuit, int& outRank) {
    if (input.size() != 2) return false;

    char rankChar = input[0];
    char suitChar = std::toupper(static_cast<unsigned char>(input[1]));
    
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

int main() {
    std::vector<Tile> hand;
    hand.reserve(14);

    std::cout << "Welcome to Mahjong Analyzer!\n";
    std::cout << "Enter 14 numbered tiles:\n";

    while (hand.size() < 14) {
        std::string input;

        if (!(std::cin >> input)) {
            std::cout << "\nInput stream closed. Exiting.\n";
            return 1;
        }

        Suit parsedSuit;
        int parsedRank;

        if (!parseTile(input, parsedSuit, parsedRank)) {
            std::cout << "Invalid tile. Retry. \n";
            continue;
        }

        hand.emplace_back(parsedSuit, parsedRank);
    }
    
    std::cout << "Your hand: ";

    for (int i = 0; i < 14; i++) {
        std::cout << hand[i].rank
                  << suitCharConv(hand[i].suit) << " ";
    }

    std::cout << "\n";

    return 0;
}
