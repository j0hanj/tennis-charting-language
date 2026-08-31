# Tennis Charting Language (`tcl`)

A from-scratch **compiler front-end for tennis**.

The [Match Charting Project](https://github.com/JeffSackmann/tennis_MatchChartingProject)
(Tennis Abstract / Jeff Sackmann) stores thousands of professional matches as a
terse shot-by-shot shorthand. One point looks like this:

```
4ffbbf*
```

> serve out **wide** → forehand → forehand → backhand → backhand → forehand **winner**

The data is rich but there is no rigorous, well-tested library that turns a
notation string into a validated, structured model of the point. `tcl` is that
library and a CLI on top of it.

## What it does

| Stage | What happens |
|-------|--------------|
| **Lex** | split `4ffbbf*` into tokens: `serve-wide`, `forehand`, …, `winner` |
| **Parse** | build a syntax tree: `Serve → Rally[Shot…] → Outcome` (hand-written recursive descent, no parser generator) |
| **Analyze** | check the point is *legal*: players alternate, only one terminal marker, running score matches the recorded final score. Emit compiler-style diagnostics pointing at the offending character. Flags real charting errors in the dataset. |
| **Lower** | flatten the tree into a columnar shot table (one row per shot) |
| **Report** | rally-length distributions, serve+1 patterns, shot tolerance vs rally length, serve-placement effectiveness, pressure-point splits |
| **Visualize** | render a point to an animated SVG of the ball moving around the court |

A separate warm-up module (`src/scoring/`) is a self-contained tennis **scoring
state machine** (deuce, tiebreaks, the historically shifting Wimbledon final-set
rules) with a small model checker that proves no impossible score is reachable.
The analyzer reuses its score logic.

## Build

Requires a C++20 compiler and CMake ≥ 3.20.

```sh
cmake -B build
cmake --build build
ctest --test-dir build
./build/tcl --version
```

## Status

Early scaffold. See the roadmap below; each box is roughly one sitting.

### Warm-up — scoring engine
- [x] `Score` type + `step()` transition for best-of-3, no tiebreak
- [ ] tiebreaks, best-of-5, `MatchFormat` as data
- [ ] per-format final-set rules (Wimbledon pre-2019 / 2019–21 / 2022+, US Open, AO)
- [ ] BFS model checker + invariants (valid / monotonic / terminating / single winner)
- [ ] property tests + state-graph image → **v0.1**

### Flagship — notation toolkit
- [ ] `docs/notation.md`: EBNF grammar (serve + core shots + directions + ends)
- [ ] lexer + token tests + error recovery on unknown characters
- [ ] recursive-descent parser for one point → AST, golden tests
- [ ] `render_diagnostic()` — underlined compiler-style errors
- [ ] CSV driver: read an MCP `-points` file into `Point`s
- [ ] sema: player alternation + assignment
- [ ] sema: serve/return legality + terminal-marker rules
- [ ] sema: score reconstruction vs `-matches` metadata → **v0.2**
- [ ] IR lowering to shot table + `--emit=json` / `--emit=csv`
- [ ] analytics: rally length, serve+1, direction tendencies
- [ ] analytics: shot tolerance, serve effectiveness, pressure splits
- [ ] differential test vs published `-stats` files
- [ ] `tcl viz` SVG emitter + web canvas demo + README GIF → **v1.0**

## License

Code: MIT (see `LICENSE`). Match Charting Project data: CC BY-NC-SA 4.0, not
redistributed here — see `NOTICE` and `scripts/fetch_mcp_data.sh`.
