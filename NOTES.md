# notes

running log so i remember what i did and what's next.

## day 1
repo setup. cmake, a cli that just prints a version, catch2 wired in for tests.
wrote up the notation from the tennis abstract quick start guide into
docs/notation.md. had to park the github actions yml as a .txt because my gh
token doesn't have workflow scope, will fix later.

## day 2
scoring state machine. `step(score, whoWonThePoint)` -> new score. did:
- points 0/15/30/40
- deuce + advantage
- games, win by 2 from 6
- sets, best of 3
no tiebreak yet. at 6-6 it just keeps going (7-5, 8-6, whatever).

the deuce check got me for a bit — i was testing for exactly 40-40 to start
counting advantage, but the clean way is "someone has 4+ points and is ahead by
2 = game over", and everything else falls out of that. redid it that way.

tests are in test/unit/scoring_test.cpp. haven't actually run them, need to
install cmake on this laptop. compiled score.cpp on its own with clang to make
sure it builds.

## day 3
tiebreak. put it behind a `has_tiebreak` flag in MatchFormat so the plain
advantage-set default doesn't change. at 6-6 games the point rules switch to
first-to-7-win-by-2, and winning the breaker wins the set no matter the game
margin (7-6). game_score shows raw numbers like "6-6" during a breaker instead
of 40-30 stuff.

didn't do serve rotation inside the breaker yet (the 1 then 2-2-2 thing) - the
server field still just flips per game. fine for now, the point winner is all
that matters for the score.

wrote a helper in the test that alternates games to get to exactly 6-6, because
if one player just wins 6 straight the set's already over at 6-0 (obviously).
tripped over that for a sec.

## day 4
small cli command, `tcl score aabba`, replays point winners and prints the score
line by line. mostly so i can eyeball the scoring engine without writing a test
every time. `--tb` flag for the tiebreak format.

then did serve rotation in the breaker. it's 1 point then 2 at a time,
alternating. added `current_server()` that works it out from how many points
have gone - the stored server field stays as the guy who serves point 1, and
the flip at the end of the breaker already lands on the right person for the
next set (whoever served first in the breaker receives first after).

## day 5
best of 5 was tiny, just sets_to_win = 3, added the factory funcs + a --bo5 flag.

then the lexer. `lex()` walks a charting string one char at a time and spits out
tokens - shot / digit / position / end-marker / error-loc, plus unknown for
anything else. digits (serve dir, rally dir, return depth) all come out as one
kind with a value since which one it is depends on where it sits, that's the
parser's problem. unknown chars get a diagnostic with the offset and it keeps
going instead of bailing. `tcl lex 4ffbbf*` dumps the tokens.

didn't overthink the token type - everything's a single char right now so text
is just a 1-char string_view into the source. if multi-char markers show up
later (serve and volley etc) the shape still works.

## day 6
the parser. recursive descent, one point string in, a tree out
(serve / rally of shots / outcome). `src/ast/` has the node structs,
`src/parser/` does the walking.

structure of a point string turned out simple once i stopped overthinking it:
serve direction digit, then shots, then an ending marker. a shot is a letter
then any mix of a direction digit (1-3), a depth digit (7-9) and a position
mark (+ - =) in whatever order - the doc examples put them in different orders
so i just consume the run and sort each piece by what it is.

drops the kUnknown tokens before parsing so the lexer's the only thing that
complains about weird characters - otherwise one bad char threw like three
errors. everything returns a best-effort tree even on bad input, no exceptions.
`tcl parse 4ffbbf*` prints it.

second serves / double faults aren't in here - the mcp csv keeps 1st and 2nd
serve in separate columns so that's a level up, once i'm reading the csv.

## day 7
court drawing. `src/viz/` takes a parsed Point and writes an svg - the court
(singles, real proportions, 9px a foot), then the ball path as a polyline
bouncing end to end. serve direction picks the service box spot, shot direction
1/2/3 -> left/mid/right, depth 7/8/9 -> how far back. blue dot on the serve,
gold dot on a winner, red x where an error went (net / wide / deep from the
error location code).

it's a schematic, not real tracking - the notation doesn't have coordinates, so
i'm not pretending. put four rendered ones in docs/examples and one in the
readme. `tcl viz "4ffbbf*" -o point.svg`.

## day 8
caret error output. `render_diagnostic()` in src/lexer/ prints the source with
a `^` under the bad offset, compiler style:

    4fQf*
      ^ don't recognize 'Q'

offsets past the end (like "expected an ending") clamp to just after the last
char so they still point somewhere. `parse`, `lex` and `viz` all use it now
instead of the plain "at N: msg" line.

## day 9
csv reader. `src/match/` has a small csv line splitter (handles quoted fields,
`""` escapes) and `read_points_csv()` that reads a match charting project
"-points" file by column name, so it doesn't care about column order and
ignores columns it doesn't need. only required column is "1st". bad numbers or
a missing header get logged as an error but don't drop the row/file - same
"one bad thing shouldn't kill the rest" idea as the lexer.

pulled 10 real rows from the actual dataset for `test/corpus/sample-points.csv`
and ran `tcl points` on them, and immediately found a real bug: my notation
docs had the error-location letter coming *after* the `@`/`#`/`*` marker
(`...@n`), but real charted points put it *before* (`...n@`). half the sample
rows failed to parse because of it. flipped `parse_ending()` to check for the
error-loc token before the end marker instead of after, fixed notation.md, and
now only 2 of the 10 real rows have anything to say - both are shot codes i
haven't added yet (`;` and `^` for let cords, `j` - not sure what that one is
yet), which is expected, they're still on the "not handling yet" list.

`tcl points match.csv` reports rows / points parsed / how many the parser
flagged.

## day 10
sema, the actual point of this project: does the charted match make sense.
`src/sema/reconcile.cpp` replays every row's PtWinner through the scoring
engine and checks it against the row's own Pts column.

took a minute to figure out what Pts actually means. it's not "player 1 - player
2", it's **server first** - whoever's serving that point has their count listed
first, no matter which player that is. so when the server is player 2 i swap my
internal score before comparing. also AD-40 in the file vs the Ad-40 i render -
just compare case-insensitive, not worth caring about.

only handles best of 3 with a standard tiebreak so far (same gap as scoring).
if a match needs different rules the replay finishes "early" - instead of
spamming an error for every row after that, it logs one and stops checking
that match_id, picks back up clean on the next one.

swapped `test/corpus/sample-points.csv` to points 1-12 of a match instead of
a random slice from the middle - a middle slice starts mid-game with no way to
know the real starting score, so reconciliation couldn't check anything
useful. points 1-12 from the start of a real match now lint completely clean:
0 parse problems, 12/12 scores match.

new `tcl lint file.csv` - runs points plus the score check, one command.

also checked the Svr column while i was in there, since i already had
`current_server()` sitting around from the tiebreak work. free bonus: this is
the first time that serve-rotation logic has been checked against a real
match instead of just my own hand-built test cases, and it held up.

one snag: nothing in the row data says who serves *game one* - a match can
start with either player serving, it's not always player 1. so the engine now
seeds who's serving from the very first row of each match and checks
everything after that against the replay, instead of always assuming player 1
opens. had to fix a couple existing tests that only worked by accident because
they never questioned who served first.

## day 11
whole-match shot chart. `render_match_svg()` reuses everything from the single
point drawer (pulled the shared bit into `compute_point_render()` so both
functions use it) but plots every point's bounces on one court instead of
drawing a path per point, translucent so it builds up where shots cluster.
`tcl matchviz file.csv -o shots.svg`.

downloaded a real full match (141 points) to try it on. first version looked
bad - the direction/depth codes only give about 9 rough spots per side, so
hundreds of shots landing in the same zone just stacked into a rigid dot grid,
not a shot chart. fixed it by nudging every dot with a small deterministic
jitter (seeded off the point/shot index, so the same file always draws the
same picture, it's not random each run). looks a lot more like a real heatmap
now instead of a checkerboard.

had to fix the title too - a match_id can be way longer than the ~310px wide
court, first render just ran the text straight off the edge. clips long
titles with an ellipsis now.

double checked the refactor didn't change the old single-point output -
diffed the four svgs already in docs/examples against freshly generated ones,
byte-identical.

## day 12
big one. flattened a match into one row per shot (`src/ir/`) and built the stats
on top (`src/analytics/`): `tcl stats match.csv` prints serve numbers, rally
length histogram, how points end by rally length, serve direction win rates.
`tcl shots` dumps the flat table as csv.

to get who-hit-what i just alternate: server serves, returner hits shot 2, and
so on. which gave me a second check for free - the shot string and the PtWinner
column are two separate records of who won the point. an ace should go to the
server, a netted forehand should go to whoever *didn't* hit it. if they
disagree one of them is a charting mistake. that's the new part of `tcl lint`.

then i ran it on the whole 2020s file for the first time (547k points, 3,337
matches). 2 seconds, no crashes, and it found a bunch of stuff:

- **the file isn't always in point order.** ~600 matches have a chunk of later
  points sitting *before* the start (one had points 92-141 first, then 1-91),
  probably charted in two sessions. replaying rows as-is scored nonsense.
  reader sorts by Pt within each match now, line_no still points at the real
  line. that one bug was making a full match show 100 score errors, now 0.
- **the letters i was dropping were shots.** first pass had 1,221 points where
  the shot string disagreed with PtWinner. grepped what the most common
  "unrecognized" letters were actually doing: `m` is the backhand lob (it's in
  the quick start guide), `j`, `k`, `t`, `q` all show up as rally shots with a
  direction like any other, and dropping them shifted who-hit-what by one.
  added them -> 60 disagreements across all 547k rows. those 60 are probably
  real charting mistakes, want to go look at some.
- `c` at the very start (`cc4f18f...`) is a let on the serve, one per c.
- `+` right after the serve digit is serve-and-volley (~23k points). before, it
  cut the rally off and those points parsed as having no shots.
- a serve that missed (`4w`, `6d`) is valid, it's just direction + fault code
  with no ending marker. lint was calling every fault an error.
- an error marker on a bare serve (`4#`) is the *returner's* error, not the
  server's. that one i had backwards at first.
- most Grand Slam matches are best of five and were getting replayed as best of
  three. each match tries bo3 first and falls back to bo5 if it fits better.

where it lands: **3,280 of 3,337 matches (98%) replay perfectly** - score and
server both, start to finish. parse problems 64,790 -> 31,320.

the 57 that don't are almost all NextGen Finals, which has its own scoring
(first to 4 games, no-ad) - that's exactly what MatchFormat is for, so that's
the next thing.

also: never actually ran the test suite until today. no cmake here, so i wrote
a throwaway catch2 stand-in and ran the real test files against it. found two
tests i'd already committed that were just wrong (one could never finish - a
15/15 split in an advantage set never ends; one still had the old
before-i-seeded-the-server version). fixed. 89 cases, 890 checks, all pass.

## day 13
nextgen finals. added `no_ad` to MatchFormat (at deuce the next point just wins,
`points[w] >= points_to_win_game` with no margin check) and a `nextgen_finals()`
factory: best of 5, first to 4 games, breaker at 3-3, no-ad. the breaker/set-won
logic didn't need to change at all - games_for_tiebreak was already a format
field, not hardcoded to 6, so pointing it at 3 just worked.

reconcile now tries bo3 -> bo5 -> nextgen finals per match and keeps whichever
has the fewest issues. matches that replay cleanly: 3,280 -> 3,292 (98%, same
percentage but a cleaner 98% - the nextgen ones aren't being scored wrong
anymore, they're just gone from the list). score issues across the whole file
dropped from 2,104 to 386.

the 45 matches still off aren't nextgen anymore - probably real deciding-set
tiebreak variations (wimbledon/ao have changed their final-set rules a few
times), or retirements/walkovers where the charted points don't run to a normal
finish. that's the actual next thing on the list.

## next
- the inline marks: `;` `^` `!` are ~30k of the remaining parse problems
- go look at the 60 shot-string-vs-PtWinner disagreements, see if they're real
- the final-set rules per tournament. wimbledon especially - advantage set
  before 2019, then 12-12 tiebreak, then 10-point tiebreak at 6-6 from 2022.
  MatchFormat should hold the rules so step() doesn't turn into a pile of ifs
- stats off the shot table, once there's an IR to flatten into
