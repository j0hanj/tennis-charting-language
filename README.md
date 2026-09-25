# tcl

messing around with tennis match data.

people chart pro matches shot by shot, and the format everyone uses (the match
charting project, off tennis abstract) is this shorthand where a whole point is
one string:

```
4ffbbf*
```

serve out wide, forehand, forehand, backhand, backhand, forehand winner.

`tcl viz` draws one point, `tcl matchviz` plots every shot from a whole match
on one court (rough - the notation only gives loose directions, this is a feel
for where things happened, not real tracking):

<img src="docs/examples/match-shot-chart.svg" width="280" alt="every shot from a real charted match, plotted on one court">

more in [docs/examples](docs/examples). `tcl stats` on that same match:

```
Jesper De Jong vs Michael Zheng  (141 points)

                        Jesper De Jong  Michael Zheng
  points won            64              77
  first serve in        60%             56%
  won on serve          58%             70%
  aces                  6               3
  double faults         4               2
  ...
```

there's thousands of matches typed out like this and basically nothing that reads
it properly - people load it into a spreadsheet or write a one-off script and
move on. so this is me building an actual parser for it, plus some stats on top.
eventually i want it to:

- read a point string into something structured
- read a whole charted match csv and check it's not broken - score doesn't add
  up, players not alternating, impossible shot order
- flag the actual charting mistakes in the data, there's a fair few
- rally length distributions, what a guy does on the ball right after his serve,
  error rate vs how long the rally goes, serve placement on break points
- draw a point - animate the ball around the court

## right now

pretty bare. what's there:

- scoring state machine (`src/scoring/`) - feed it point winners and it tracks
  the score. needed because the shorthand only records who won each point, not
  the score, so to check a match you replay it and see if the final score lines
  up. points, deuce, games, sets, best of 3 and 5, tiebreak with the serve
  rotation
- lexer (`src/lexer/`) - turns a charting string into tokens, flags characters
  it doesn't know and keeps going
- parser (`src/parser/`, `src/ast/`) - hand-written recursive descent, tokens ->
  a tree for one point (serve, rally of shots with direction/depth/position, how
  it ended). partial tree + diagnostics on bad input, doesn't throw. errors
  print with a caret under the bad character:

  ```
    4fQf*
      ^ don't recognize 'Q'
  ```
- viz (`src/viz/`) - `tcl viz "4ffbbf*" -o point.svg` draws one point;
  `tcl matchviz match.csv -o shots.svg` plots every shot from a whole file on
  one court, jittered so hundreds of shots landing in the same rough zone
  don't just stack into a grid. schematic, positions are approximate
- csv reader (`src/match/`) - reads a match charting project "-points" csv by
  column name (order/extra columns don't matter). `tcl points match.csv` runs
  every point through the parser and reports how many it had something to say
  about
- sema (`src/sema/`) - replays each row's PtWinner through the scoring engine
  and checks it against the file's own Pts (server-first) and Svr columns -
  the Svr check doubles as validation of the tiebreak serve rotation against
  real data. who serves game one is taken from the first row, everything
  after that is checked, not assumed. this is the actual "does this charted
  match make sense" check. also checks the shot string agrees with the PtWinner
  column about who won (an ace should go to the server, a netted forehand to
  the other guy)
- ir + stats (`src/ir/`, `src/analytics/`) - flattens a match into one row per
  shot (who hit it, type, direction, depth, how the point ended), then
  `tcl stats match.csv` works out serve numbers, rally lengths, and how points
  end by rally length. `tcl shots match.csv -o shots.csv` dumps the flat table.
  sample output in [docs/examples/stats-example.txt](docs/examples/stats-example.txt)
- `tcl score aabba` (`--tb`, `--bo5`), `tcl lex 4ffbbf*`, `tcl parse 4ffbbf*`,
  `tcl viz 4ffbbf*`, `tcl points file.csv`, `tcl lint file.csv`,
  `tcl matchviz file.csv`, `tcl stats file.csv`, `tcl shots file.csv` - mostly
  for eyeballing while building
- notes on the notation in `docs/notation.md`

## todo

- [x] scoring: points, deuce, games, sets, best of 3
- [x] scoring: tiebreak
- [x] scoring: serve rotation in the breaker
- [x] scoring: best of 5
- [x] scoring: short sets / no-ad (nextgen finals)
- [x] scoring: final-set match-tiebreak (wimbledon / AO style, first to 10)
- [ ] scoring: walk every reachable score, make sure none are impossible
- [ ] write out the notation grammar properly
- [x] tokenizer
- [x] parser -> tree for one point
- [x] error messages that point at the bad character (caret under the offset,
      like a compiler)
- [x] read a full match csv
- [x] the checks (replay the score and server, shot string vs PtWinner)
- [x] flatten to one row per shot
- [x] stats (first pass - serve numbers, rally lengths, endings)
- [x] the court drawing (schematic for now, no real coordinates)
- [x] whole-match shot chart (`tcl matchviz`)

## how it holds up on the real data

ran it over the whole 2020s charting file (547k points, 3,337 matches, ~2
seconds). 3,333 of the matches (99%) replay perfectly through the scoring engine
- score and server match the file's own columns start to finish, trying best of
3, best of 5, NextGen Finals (short sets, no-ad), and the wimbledon/AO-style
final-set breaker (first to 10 instead of playing out the last set) per match.
the shot strings agree with the PtWinner column on all but 60 of the 547k
points, which are probably real charting mistakes.

## build

need cmake + a c++20 compiler. catch2 gets pulled in for tests.

```
cmake -B build
cmake --build build
ctest --test-dir build
```

## data

the match charting data isn't mine - it's jeff sackmann / tennis abstract
(CC BY-NC-SA). i don't check it in, `scripts/fetch_mcp_data.sh` grabs it. the
notation writeup in `docs/` is my own, from their guide.
