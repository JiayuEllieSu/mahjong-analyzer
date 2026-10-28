#include <iostream>
#include <string>
#include <vector>

struct Tile {
    char suit;
    int rank;
};

int main() {
    std::vector<Tile> hand;

    std::cout << "Welcome to Mahjong Analyzer!\n";
    std::cout << "Enter 14 numbered tiles:\n";

    while (hand.size() < 14) {
        std::string input;
        std::cin >> input;

        // Check that the input is valid
        if (input.size() != 2 ||
            input[0] < '1' || input[0] > '9' ||
            (input[1] != 'B' &&
             input[1] != 'D' &&
             input[1] != 'C')) {

            std::cout << "Invalid tile. Try again.\n";
            continue;
        }

        Tile tile;

        tile.rank = input[0] - '0';
        tile.suit = input[1];

        hand.push_back(tile);
    }

    std::cout << "Your hand: ";

    for (int i = 0; i < 14; i++) {
        std::cout << hand[i].rank
                  << hand[i].suit << " ";
    }

    std::cout << "\n";

    return 0;
}
