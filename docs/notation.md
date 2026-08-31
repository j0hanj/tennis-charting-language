# The Match Charting Project shorthand — notation reference

This is an independent description of the charting shorthand used by the
[Match Charting Project](https://github.com/JeffSackmann/tennis_MatchChartingProject),
derived from the public Tennis Abstract "Quick Start Guide" and the MatchChart
instructions sheet. It is the spec `tcl`'s lexer and parser implement.

> **Status:** draft. Codes below are confirmed; a formal EBNF grammar and the
> full code tables (rare shots, net cords, serve-and-volley, unusual endings)
> are still to be written. v1 targets singles with the core codes only.

## A point string, left to right

A point is recorded as one string: the serve, then each shot of the rally in
order, then how the point ended.

```
4 f f b b f *
| \___________/ \
serve   rally    ending
```

Example: `4ffbbf*` = serve wide, then forehand, forehand, backhand, backhand,
forehand winner.

## Serve direction (first shot only)

| Code | Meaning        |
|------|----------------|
| `4`  | wide           |
| `5`  | body           |
| `6`  | down the T     |

A faulted first serve is recorded, followed by the second serve; a double fault
ends the point. (Exact fault/ace/unreturned encoding: TODO.)

## Shot types

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
| `l`  | lob                      |
| `u`  | forehand drop shot       |
| `y`  | backhand drop shot       |
| `h`  | forehand half-volley     |
| `i`  | backhand half-volley     |

(List to be completed and verified against the instructions sheet. Unknown
letters are a lexer error.)

## Shot direction (rally shots)

| Code | Meaning                                            |
|------|---------------------------------------------------|
| `1`  | to a right-hander's forehand corner (lefty: BH)   |
| `2`  | down the middle                                   |
| `3`  | to a right-hander's backhand corner (lefty: FH)   |

## Return depth (service return only)

| Code | Meaning                          |
|------|----------------------------------|
| `7`  | shallow (within the service boxes) |
| `8`  | moderately deep (behind service line) |
| `9`  | very deep (back quarter of the court) |

## Court position markers

| Code | Meaning        |
|------|----------------|
| `+`  | approach shot  |
| `-`  | shot hit at the net |
| `=`  | shot hit at the baseline |

## Point endings

| Code | Meaning        |
|------|----------------|
| `*`  | winner         |
| `@`  | unforced error |
| `#`  | forced error   |

Error location (follows `@` or `#`):

| Code | Meaning         |
|------|-----------------|
| `n`  | into the net    |
| `w`  | wide            |
| `d`  | deep            |
| `x`  | wide and deep   |

## Worked example

```
5 r 3 7 b + 3 l 2 o = 1 r #
```

body serve; slice return to the backhand corner, shallow; backhand approach to
the backhand corner; lob down the middle; overhead from the baseline to the
forehand corner; forehand forced error.

## Not yet handled (deferred past v1)

- doubles
- let cords / net cords
- serve-and-volley marker
- unusual endings (shank, foot fault, hindrance, retirement)
- rally annotations for time / court position beyond `+ - =`
