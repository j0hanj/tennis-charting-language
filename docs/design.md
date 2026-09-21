# how it's put together

the charting string goes through a few stages, each small enough to test on its
own. it ends up looking like a compiler front end.

```
"4ffbbf*"
  -> lexer     split into tokens (serve-wide, forehand, ..., winner)
  -> parser    build a syntax tree: Serve -> [Shot, Shot, ...] -> Outcome
  -> sema      check it's legal, figure out who hit what, rebuild the score
  -> ir        flatten the tree into one row per shot
  -> stats / viz / json export all read the flat rows
```

## folders under src/

- `scoring/` - keeps score. standalone, nothing depends on the parser. sema uses
  it to check the score in a charted match adds up
- `lexer/` - string -> tokens. if it hits a character it doesn't know it records
  an error and keeps going instead of blowing up
- `ast/` - the node types for a parsed point, just plain structs
- `parser/` - hand written recursive descent, one point at a time. no yacc/antlr,
  the whole point is to write it myself
- `sema/` - the checks. replays the score and the server against the file's own
  columns, and checks the shot string agrees with PtWinner about who won
- `match/` - reads the csv. puts rows in point order per match, because the real
  files aren't always in order
- `ir/` - flatten every point into a summary plus one row per shot (who hit it,
  type, direction, depth, how the point ended). `tcl shots` dumps it as csv
- `analytics/` - the stats, all computed off the flat rows. names come out of
  the match_id
- `viz/` - map shots to court coordinates, write an svg. no image library
- `cli/` - arg parsing and glue

## a couple of things i'm trying to stick to

- match formats and the code tables are data, not if-chains, so adding a rule or a
  new shot code is just adding an entry
- the ast doesn't get mutated, passes add info alongside it
- stats and viz only ever touch the flat ir rows, never the tree
- one bad point shouldn't kill a whole match file
