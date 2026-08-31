# tcl — tennis charting language

i watch a lot of tennis and got into charting matches. the notation everyone uses
(from tennis abstract's [match charting project](https://github.com/JeffSackmann/tennis_MatchChartingProject))
is this dense shorthand where a whole point is one string:

```
4ffbbf*
```

serve out wide, forehand, forehand, backhand, backhand, forehand winner.

there are thousands of pro matches typed out in that format and basically no code
that reads it properly. so i'm writing a parser for it, plus some analysis on top.
the notation is basically a tiny language so i'm building it like a compiler front
end — tokenizer, parser, a checker, then stats. figured it's a better way to
actually learn how parsers work than just reading about them.

## what it should do when it's further along

- `tcl lint match.csv` — read a charted match, tell you if it's broken: score
  doesn't add up, players not alternating, impossible shot sequence. the charting
  data has real mistakes in it and this should catch them
- `tcl stats match.csv` — rally length distributions, what a player does on the
  shot right after their serve, error rate vs how long the rally is, serve
  placement on break points
- `tcl viz "4ffbbf*"` — draw the point, animate the ball around the court
- `tcl parse "4ffbbf*"` — dump the parsed point as json

## where it's at right now

early. so far:

- **scoring state machine** (`src/scoring/`) — keeps score through a match.
  needed because the notation only records who won each point, not the score, so
  to check a match you replay every point and see if the final score matches the
  record. points / deuce / advantage / games / sets / best of 3 work. no tiebreak
  yet, if it hits 6-6 it just keeps going
- cli that does `--version` and not much else
- my notes on the notation in `docs/notation.md`

see `NOTES.md` for the running log and `docs/design.md` for how the pieces fit.

## todo

- [x] scoring: points, deuce, games, sets, best of 3
- [ ] scoring: tiebreak
- [ ] scoring: best of 5
- [ ] scoring: the wimbledon final-set rule changes (they changed it twice)
- [ ] scoring: walk every reachable score, check none are impossible
- [ ] notation: write out the grammar properly
- [ ] lexer
- [ ] parser -> syntax tree for one point
- [ ] nice error messages that point at the bad character
- [ ] read a full match csv
- [ ] the semantic checks (alternation, score reconstruction)
- [ ] flatten to a shot table
- [ ] stats
- [ ] the court visualizer

## building

need cmake (>= 3.20) and a c++20 compiler.

```
cmake -B build
cmake --build build
ctest --test-dir build
```

## data

the match charting data isn't mine — it's jeff sackmann / tennis abstract
(CC BY-NC-SA). i don't check it into the repo, `scripts/fetch_mcp_data.sh` pulls
it into `data/`. the notation writeup in `docs/` is my own, from their guide.
