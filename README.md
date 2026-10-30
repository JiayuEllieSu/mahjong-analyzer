# American Mahjong Analyzer

A C++ project designed to analyze American Mahjong hands and eventually recommend optimal moves using combinatorics, probability, and search algorithms.

## Overview

The goal of this project is to develop a Mahjong decision engine that evaluates a player's hand, identifies possible winning combinations, and recommends strategic moves.

## Run

From the repository folder:

```bash
clang++ -std=c++20 -Wall -Wextra main.cpp -o mahjong
./mahjong
```

Enter 14 tiles, separated by spaces. The program prints tile counts and a ranked list of discards.

## First analysis algorithm

- `hand_analyzer.hpp`: matches the hand to target groups (singles, pairs, pungs, kongs, quints) and ranks possible discards.
- `demo_patterns.txt`: invented examples **not taken from the official yearly card**.
- `tests/test_analyzer.cpp`: small correctness tests.

Pattern-file format:

```text
Name | 2*F 3*5B 3*6B 3*7B 3*8B
```

Every pattern must total 14 tiles. Write each single or pair as its own group, because jokers are allowed only in sets of three or more identical tiles. If an NMJL card line allows several suit or number choices, list each concrete variation separately for now.

Discards are ranked first by **fewest tiles missing** from any entered pattern, then by **number of unseen copies of improving draws**. Unseen counts currently subtract only your hand; exposed and discarded tiles can be supplied to the engine later. This is not yet a complete American Mahjong strategy or an estimate of actual win probability. Calls, exposures, wall composition, opponent strategy, card scoring, and the current year's official card are not yet modeled.

To run the tests:

```bash
clang++ -std=c++20 -Wall -Wextra tests/test_analyzer.cpp -o test_analyzer
./test_analyzer
```
