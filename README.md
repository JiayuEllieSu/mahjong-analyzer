# American Mahjong Analyzer

A C++17 command-line tool that analyzes 14-tile American Mahjong hands, ranks discards, and identifies useful draws.

## Build and test

```bash
make
make test
```

## Usage

```bash
./mahjong
```

Analyze a hand directly:

```bash
./mahjong --hand "F F 5B 5B 5B 6B 6B 6B 7B 7B 7B 8B 8B E"
```

Include visible tiles and show all discard options:

```bash
./mahjong --hand "F F 5B 5B 5B 6B 6B 6B 7B 7B 7B 8B 8B E" \
  --visible "8B J J J J" --all
```

Use custom patterns:

```bash
./mahjong --patterns my_patterns.txt --hand "..."
```

Run `./mahjong --help` for all options.

## Tile codes

- **Suits:** `1B–9B`, `1D–9D`, `1C–9C`
- **Winds:** `E S W N`
- **Dragons:** `RD GD WD`
- **Special:** `F` (flower), `J` (joker)

Enter exactly 14 tiles. Codes are case-insensitive.

## Patterns

Each line in the pattern file defines a concrete 14-tile hand:

```text
Bamboo pungs with flowers | 2*F 3*5B 3*6B 3*7B 3*8B
```

Jokers can substitute only in groups of three or more. All permitted patterns must be explicitly listed.

## Discard rankings

Discards are ranked by:

1. Fewest tiles missing from a target pattern.
2. Most available improving tiles.

The analyzer also displays the tiles needed to complete a pattern and an estimated improvement percentage.

**Note:** Probabilities assume a uniform pool of unseen tiles, not the actual wall. Opponent hands, exposed-set constraints, and other gameplay factors are excluded.
