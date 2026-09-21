# notes on the match charting notation

my writeup of how the match charting project shorthand works, from the tennis
abstract quick start guide and the matchchart instructions sheet. this is what
the lexer/parser are going to follow.

still a draft. the codes here are the ones i'm sure about. the rarer stuff (net
cords, serve and volley, weird endings) and a proper grammar are still todo. for
a first version i'm only doing singles and the common codes.

## a point string, left to right

a point is one string: the serve, then each shot in order, then how it ended.

```
4 f f b b f *
| \___________/ \
serve   rally    ending
```

example: `4ffbbf*` = serve wide, then forehand, forehand, backhand, backhand,
forehand winner.

## serve direction (first shot only)

| Code | Meaning        |
|------|----------------|
| `4`  | wide           |
| `5`  | body           |
| `6`  | down the T     |

### a serve that missed

a fault is just the direction and where it went, no ending marker: `4w` (wide
serve, missed wide), `6n` (down the T, into the net), `6d` (deep). the first
serve's fault goes in the `1st` column and the second serve in `2nd`. two faults
in a row is a double fault and the returner gets the point.

### lets

a `c` in front of the serve digit is a let (`c4b27f3*`), one `c` per let
(`cc4f18...`). they don't change who serves or who's hitting, the point just
carries on. found by grepping the real data for what the most common
"unrecognized" letter was doing - it only ever shows up at the start.

### serve and volley

a `+` right after the serve digit (`4+b1v1n#`) means the server followed the
serve in. same symbol as the approach-shot marker, it just sits right after the
serve digit instead of after a shot. i'm going off what the real data looks like
here (it's really common - about 23k points) rather than the docs.

### a serve with no rally

- `4*` - ace
- `4#` / `4@` - the serve landed and the return isn't charted. the error
  marker is the *returner's* - they couldn't get it back (forced) or blew an
  easy one (unforced), so the server wins the point. this one caught me out, my
  first version blamed the server

## shot types

| Code | Shot                     |
|------|--------------------------|
| `f`  | forehand (topspin/flat)  |
| `b`  | backhand (topspin/flat)  |
| `r`  | forehand slice           |
| `s`  | backhand slice           |
| `v`  | forehand volley          |
| `z`  | backhand volley          |
| `o`  | forehand overhead / smash|
| `p`  | backhand overhead        |
| `l`  | forehand lob             |
| `m`  | backhand lob             |
| `u`  | forehand drop shot       |
| `y`  | backhand drop shot       |
| `h`  | forehand half-volley     |
| `i`  | backhand half-volley     |

still filling this in from the instructions sheet. a letter thats not in this list is a lexer error.

`m` (backhand lob) is from the quick start guide. `j` and `k` show up as rally
shots all through the real data (`3s2j=`) so the lexer treats them as shots -
i think they're the forehand/backhand drive volleys but i haven't confirmed that
against the instructions sheet yet. `t` (trick shot, i think) and `q` (a shot
whose type wasn't identified) also sit in rallies with a direction like any other
shot, so they count.

there are probably more codes i haven't run into yet.

## shot direction (rally shots)

| Code | Meaning                                            |
|------|---------------------------------------------------|
| `1`  | to a right-hander's forehand corner (lefty: BH)   |
| `2`  | down the middle                                   |
| `3`  | to a right-hander's backhand corner (lefty: FH)   |

## return depth (on the return only)

| Code | Meaning                          |
|------|----------------------------------|
| `7`  | shallow (within the service boxes) |
| `8`  | moderately deep (behind service line) |
| `9`  | very deep (back quarter of the court) |

## court position markers

| Code | Meaning        |
|------|----------------|
| `+`  | approach shot  |
| `-`  | shot hit at the net |
| `=`  | shot hit at the baseline |

## how the point ended

| Code | Meaning        |
|------|----------------|
| `*`  | winner         |
| `@`  | unforced error |
| `#`  | forced error   |

error location, comes right *before* `@` or `#` - e.g. an unforced net error
ends `...n@`, not `...@n`. i had this backwards at first, only noticed when i
ran the parser against real charted rows and it kept failing right there:

| Code | Meaning         |
|------|-----------------|
| `n`  | into the net    |
| `w`  | wide            |
| `d`  | deep            |
| `x`  | wide and deep   |

## a worked example

```
5 r 3 7 b + 3 l 2 o = 1 r #
```

body serve; slice return to the backhand corner, shallow; backhand approach to
the backhand corner; lob down the middle; overhead from the baseline to the
forehand corner; forehand forced error.

## stuff im not handling yet

seen in real data, flagged as unrecognized for now:

- `;` and `^` and `!` - inline marks in the middle of a rally. they're not shots
  (they don't change who's hitting) so dropping them keeps the alternation right

and the rest of the not-yet list:

- doubles
- let cords / net cords (`;` might be this)
- unusual endings (shank, foot fault, hindrance, retirement)
- rally annotations for time / court position beyond `+ - =`
