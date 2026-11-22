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

Get helpful draw recommendations from a 13-tile hand:

```bash
./mahjong --draws --hand "F F 5B 5B 5B 6B 6B 6B 7B 7B 7B 8B 8B"
```

Export structured results for scripts or a browser frontend:

```bash
./mahjong --draws --json --hand "F F 5B 5B 5B 6B 6B 6B 7B 7B 7B 8B 8B" > draws.json
./mahjong --json --all --hand "F F 5B 5B 5B 6B 6B 6B 7B 7B 7B 8B 8B E" > discards.json
```

JSON includes the unknown-pool size, closest pattern, missing-tile count, remaining requirements, and improving draws with available copy counts. The `improvement_percentage` assumes an equally likely draw from all unaccounted-for physical tiles, not the actual wall. Use `--all` to include every discard choice in JSON.

Run `./mahjong --help` for all options.

## Tile codes

- **Suits:** `1B–9B`, `1D–9D`, `1C–9C`
- **Winds:** `E S W N`
- **Dragons:** `RD GD WD`
- **Special:** `F` (flower), `J` (joker)

Enter exactly 14 tiles for discard analysis or 13 tiles with `--draws`. Codes are case-insensitive.

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

## Unreachable patterns

The analyzer now checks **whether each pattern can still be completed** before
using it to recommend a discard. It counts exposed/discarded tiles supplied via
`--visible` and also treats the candidate discard as out of play. Singles
and pairs require natural tiles, while groups of three or more may use the
remaining jokers to cover natural-tile shortages.

For instance, if you hold the last East tile and a pattern requires an East
single, discarding East makes that pattern unreachable. A discard with no
reachable target patterns appears last and is marked `unreachable` (use
`--all` to see every option).

This is a **necessary availability check**, not a prediction of the wall:
opponents' concealed tiles and other gameplay constraints remain unknown.

**Note:** Probabilities assume a uniform pool of unseen tiles, not the actual wall. Opponent hands, exposed-set constraints, and other gameplay factors are excluded.
