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

a faulted first serve gets recorded, then the second serve. double fault ends the point. still need to nail down how aces / unreturned serves are marked.

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
| `l`  | lob                      |
| `u`  | forehand drop shot       |
| `y`  | backhand drop shot       |
| `h`  | forehand half-volley     |
| `i`  | backhand half-volley     |

still filling this in from the instructions sheet. a letter thats not in this list is a lexer error.

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

- doubles
- let cords / net cords
- serve-and-volley marker
- unusual endings (shank, foot fault, hindrance, retirement)
- rally annotations for time / court position beyond `+ - =`
