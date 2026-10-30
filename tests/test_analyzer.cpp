#include "../hand_analyzer.hpp"
#include <cassert>
#include <iostream>

int main() {
    using namespace mahjong;
    // Index 4 = 5B, 5 = 6B, 6 = 7B, 7 = 8B; 34 = flower, 35 = joker.
    Pattern pattern{"Example", {{34,2},{4,3},{5,3},{6,3},{7,3}}};
    Counts hand{};
    hand[34] = 2;
    hand[4] = hand[5] = hand[6] = hand[7] = 3;
    assert(matchPattern(hand, pattern).missing == 0);

    // A joker can complete a pung.
    hand[4] = 2;
    hand[35] = 1;
    assert(matchPattern(hand, pattern).missing == 0);

    // A joker cannot complete a flower pair.
    hand[4] = 3;
    hand[34] = 1;
    assert(matchPattern(hand, pattern).missing == 1);

    // If a target needs a pair and a pung of the same tile,
    // real tiles go to the pair and jokers fill the pung.
    Pattern overlap{"Overlap", {{4,2},{4,3},{5,3},{6,3},{7,3}}};
    Counts second{};
    second[4] = 2;
    second[5] = second[6] = second[7] = 3;
    second[35] = 3;
    assert(matchPattern(second, overlap).missing == 0);

    Counts candidate{};
    candidate[34] = 2;
    candidate[4] = candidate[5] = candidate[6] = 3;
    candidate[7] = 2;
    candidate[27] = 1; // East
    const auto choices = rankDiscards(candidate, {pattern});
    assert(choices.front().tileIndex == 27);
    assert(choices.front().missing == 1);
    assert(choices.front().improvingCopies == 10); // two 8B + eight jokers

    std::cout << "All tests passed.\n";
}
