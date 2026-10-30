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


