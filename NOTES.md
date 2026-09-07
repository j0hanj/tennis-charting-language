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

## next
- caret/underline error output (the offsets are already on every diagnostic)
- read a full match csv
- the final-set rules per tournament. wimbledon especially - advantage set
  before 2019, then 12-12 tiebreak, then 10-point tiebreak at 6-6 from 2022.
  MatchFormat should hold the rules so step() doesn't turn into a pile of ifs
