#!/usr/bin/env bash
set -euo pipefail

# Run from the repository root after compiling ./mahjong.
hand='F F 5B 5B 5B 6B 6B 6B 7B 7B 7B 8B 8B E'
output=$(./mahjong --hand "$hand" --visible '8B J J J J' \
  --patterns tests/fixture_patterns.txt --all)

grep -Fq 'Discard rankings (patterns: tests/fixture_patterns.txt)' <<< "$output"
grep -Fq 'Uniform-unknown-draw model: 133 unseen copies' <<< "$output"
grep -Fq '1. Discard E | missing: 1 | improving copies: 5' <<< "$output"
grep -Fq 'estimated improvement: 3.8%' <<< "$output"
grep -Fq 'Improving draws: J (4 left), 8B (1 left)' <<< "$output"
grep -Fq 'To finish nearest pattern: 1x8B (or J)' <<< "$output"
grep -Fq 'nearest pattern: Fixture pungs' <<< "$output"
grep -Fq '6. Discard' <<< "$output" # --all reports all six types in this hand

# Commas and lowercase tile codes are accepted.
comma_output=$(./mahjong --hand 'f,f,5b,5b,5b,6b,6b,6b,7b,7b,7b,8b,8b,e')
grep -Fq '1. Discard E' <<< "$comma_output"

# Bad hands, visibility conflicts and missing files produce a nonzero exit.
if ./mahjong --hand 'F F 5B' >/dev/null 2>&1; then
  echo 'Expected short hand to be rejected' >&2
  exit 1
fi
if ./mahjong --hand "$hand" --visible '8B 8B 8B' >/dev/null 2>&1; then
  echo 'Expected impossible tile counts to be rejected' >&2
  exit 1
fi
if ./mahjong --patterns missing-pattern-file.txt --hand "$hand" >/dev/null 2>&1; then
  echo 'Expected missing pattern file to be rejected' >&2
  exit 1
fi

echo 'All CLI tests passed.'

