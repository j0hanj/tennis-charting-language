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

## next
- serve rotation during the breaker
- best of 5
- the final-set rules per tournament. wimbledon especially — advantage set
  before 2019, then 12-12 tiebreak, then 10-point tiebreak at 6-6 from 2022.
  going to make MatchFormat hold the rules so step() doesn't turn into a pile of
  ifs
- after that: start on the actual notation parser
