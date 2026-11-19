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
    // The plan targets the exact pattern selected in the ranking.
    assert(choices.front().remainingRequirements.size() == 1);
    assert(choices.front().remainingRequirements.front().tileIndex == 7);
    assert(choices.front().remainingRequirements.front().naturalOnly == 0);
    assert(choices.front().remainingRequirements.front().jokerEligible == 1);

    // A joker cannot substitute for a missing tile in a pair.
    const auto flowerNeeds = missingRequirements(hand, pattern);
    assert(flowerNeeds.size() == 1);
    assert(flowerNeeds.front().tileIndex == 34);
    assert(flowerNeeds.front().naturalOnly == 1);
    assert(flowerNeeds.front().jokerEligible == 0);

    // When a pair and a pung use the same tile, natural tiles first fill
    // the pair; jokers can cover the remaining pung positions.
    assert(missingRequirements(second, overlap).empty());

    assert(choices.front().improvingTileTypes == 2);
    assert(choices.front().improvingDraws.size() == 2);
    assert(choices.front().improvingDraws[0].tileIndex == JOKER_INDEX);
    assert(choices.front().improvingDraws[0].remainingCopies == 8);
    assert(choices.front().improvingDraws[0].missingAfterDraw == 0);
    assert(choices.front().improvingDraws[1].tileIndex == 7);
    assert(choices.front().improvingDraws[1].remainingCopies == 2);
    assert(unseenCopies(candidate) == 138);

    Counts visible{};
    visible[7] = 1; // one 8B exposed
    visible[35] = 4; // four jokers already visible
    const auto adjusted = rankDiscards(candidate, {pattern}, visible);
    assert(adjusted.front().tileIndex == 27);
    assert(adjusted.front().improvingCopies == 5);
    assert(adjusted.front().improvingTileTypes == 2);
    assert(adjusted.front().improvingDraws[0].remainingCopies == 4);
    assert(adjusted.front().improvingDraws[1].remainingCopies == 1);
    assert(unseenCopies(candidate, visible) == 133);

    // Exhausted improving tiles should no longer count as outs.
    visible[7] = 2;
    visible[35] = 8;
    const auto exhausted = rankDiscards(candidate, {pattern}, visible);
    bool foundEast = false;
    for (const auto& choice : exhausted) {
        if (choice.tileIndex == 27) {
            foundEast = true;
            assert(choice.improvingCopies == 0);
            assert(choice.improvingTileTypes == 0);
            assert(choice.improvingDraws.empty());
        }
    }
    assert(foundEast);

    // Invalid visibility and malformed hands must fail explicitly.
    visible[7] = 3; // two 8B in hand, so three visible is impossible
    bool rejected = false;
    try { (void)rankDiscards(candidate, {pattern}, visible); }
    catch (const std::invalid_argument&) { rejected = true; }
    assert(rejected);

    Counts tooSmall{};
    tooSmall[4] = 1;
    rejected = false;
    try { (void)unseenCopies(tooSmall); }
    catch (const std::invalid_argument&) { rejected = true; }
    assert(rejected);

    std::cout << "All tests passed.\n";
}
