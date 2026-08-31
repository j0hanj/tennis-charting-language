# Design

`tcl` is structured like a compiler front-end. Source text (a charting string)
flows through stages, each with a narrow job, each independently testable.

```
charting string
      |
      v
  [ lexer ]        src/lexer/     string_view -> vector<Token> (+ diagnostics)
      |
      v
  [ parser ]       src/parser/    tokens -> AST   (hand-written recursive descent)
      |            src/ast/       Point / Serve / Rally / Shot / Outcome
      v
  [ sema ]         src/sema/      legality checks, player assignment,
      |                           score reconstruction (uses src/scoring/)
      v
  [ IR lowering ]  src/ir/        AST -> flat columnar shot table
      |
      +--> [ analytics ]  src/analytics/   distributions, patterns, splits
      +--> [ viz ]        src/viz/         shot table -> animated SVG court
      +--> [ export ]                      --emit=json / --emit=csv
```

## Modules

| Dir | Responsibility | Depends on |
|-----|----------------|-----------|
| `src/scoring/` | tennis scoring state machine + model checker (warm-up, standalone) | — |
| `src/lexer/` | tokenize one charting string; recover from unknown chars | — |
| `src/ast/` | plain-data node types for a parsed point | — |
| `src/parser/` | recursive-descent parser, one point at a time; diagnostics with byte ranges | lexer, ast |
| `src/sema/` | semantic passes over a parsed match; fills in `winner`; flags illegal/implausible sequences | ast, scoring |
| `src/ir/` | lower a validated match to a shot table (one row per shot) | ast |
| `src/analytics/` | metrics computed over the shot table | ir |
| `src/viz/` | deterministic court geometry -> SVG text (no image lib) | ir |
| `src/cli/` | argument parsing, subcommand dispatch, output formatting | all |

## Cross-cutting: diagnostics

Every stage reports problems as a `Diagnostic { severity, message, byte range }`
against the original string. `render_diagnostic()` prints the string with the
offending span underlined, compiler-style. Errors abort a single point but never
a whole match file — one malformed point should not lose the other 200.

## Principles

- **Data, not branches.** Match formats and code tables are lookup data so new
  rules/codes are additions, not `if` chains.
- **Immutable AST.** Passes read the tree and produce new information beside it.
- **Front-end / back-end split.** Analytics and viz consume the IR shot table,
  never the AST directly.
- **Every module has golden tests** from `test/corpus/` (hand-decoded points).
