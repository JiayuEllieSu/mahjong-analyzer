# American Mahjong Analyzer

A C++17 command-line analyzer for 14-tile American Mahjong hands. It compares a hand with explicitly listed 14-tile patterns, ranks possible discards, and shows which unseen tiles would improve each choice.

**Pattern warning:** The included `demo_patterns.txt` contains made-up examples, not the annual National Mah Jongg League card. Results only reflect the patterns you load; they are not official winning-hand advice.

## Build and test

From the repository root:

```bash
make
make test
```

Or build without `make`:

```bash
clang++ -std=c++17 -Wall -Wextra main.cpp -o mahjong
clang++ -std=c++17 -Wall -Wextra tests/test_analyzer.cpp -o test_analyzer
./test_analyzer
```

## Usage

Interactive input, as before:

```bash
./mahjong
```

Or supply a hand directly, which is useful for reproducing results:

```bash
./mahjong --hand "F F 5B 5B 5B 6B 6B 6B 7B 7B 7B 8B 8B E"
```

Account for exposed and discarded tiles, and see every discard option:

```bash
./mahjong --hand "F F 5B 5B 5B 6B 6B 6B 7B 7B 7B 8B 8B E" \
  --visible "8B J J J J" --all
```

Use your own pattern file:

```bash
./mahjong --patterns my_patterns.txt --hand "F F 5B 5B 5B 6B 6B 6B 7B 7B 7B 8B 8B E"
```

Tile codes are `1B` through `9B` (bamboo), `1D` through `9D` (dots), `1C` through `9C` (characters), `E S W N` (winds), `RD GD WD` (dragons), `F` (flower), and `J` (joker). Lists accept commas or spaces and tile codes are case-insensitive. Enter exactly 14 hand tiles. The optional `--visible` list contains tiles visible outside your hand. A hand and its visible tiles may not exceed four copies of a standard tile, or eight flowers or jokers.

Run `./mahjong --help` to list all options.

## Pattern file format

Each nonempty, noncomment line describes one *concrete* 14-tile pattern:

```text
Bamboo pungs with flowers | 2*F 3*5B 3*6B 3*7B 3*8B
```

Each `size*tile` token represents a group of identical tiles. Singles and pairs require natural tiles; jokers can fill spots only in groups of three or more identical tiles. In a pattern such as `NEWS`, enter each wind separately (`1*N 1*E 1*W 1*S`). Do not represent a sequence as a single group. You must explicitly enumerate permitted suit/rank combinations; this parser does not expand card templates.

## How rankings work

For each possible discard, the analyzer:

1. Finds the minimum number of tiles missing from any loaded pattern in the 13-tile hand.
2. Checks each unseen tile type to see whether drawing it decreases that minimum.
3. Reports the number of improving tile types, remaining copies of each improving tile, and their total.
4. Prints the outstanding natural-only tiles (for singles and pairs) and joker-eligible tiles (for larger groups) needed to finish the nearest concrete pattern.
5. Sorts primarily by fewer missing tiles, then by more improving copies.

The displayed improvement percentage is `improving copies / all unseen copies`. The unseen pool starts from 152 physical tiles and excludes the 14 tiles in your hand and the supplied visible tiles. **This is a simplified uniform-unknown-tile estimate, not the probability of drawing from the actual wall**. It does not account for opponents' concealed hands, drawing strategies, exposed-set constraints, joker exchanges, card-specific scoring, or the current wall. `--all` shows all discard candidates instead of the five highest-ranked ones.

**Completion plans:** `To finish nearest pattern` gives one valid allocation of held jokers to matching groups. For example, `1xF (natural only)` means a flower is required, while `1x8B (or J)` means either 8B or a joker can fill that position. If several sets could use a held joker, other allocations are also possible. The displayed requirements do not guarantee the listed tiles remain drawable; refer to improving draws and the visible-tile input for availability.
