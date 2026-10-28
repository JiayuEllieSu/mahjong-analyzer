#include <iostream>
#include <string>
#include <vector>

int main() {
    std::vector<std::string> hand;

    std::cout << "Welcome to Mahjong Analyzer!" << std::endl;
    std::cout << "Enter 14 tiles: " << std::endl;

    for (int i = 0; i < 14; i++) {
        std::string tile;
        std::cin >> tile;
        hand.push_back(tile);
    }

    std::cout << "Your hand: ";

    for (int i = 0; i < hand.size(); i++) {
        std::cout << hand[i] << " ";
    }

    std::cout << std::endl;

    return 0;
}
