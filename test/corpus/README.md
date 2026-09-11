# test corpus

hand-decoded points used as golden tests for the lexer, parser, and sema.

`sample-points.csv` is 10 real rows (one header + 10 points) from the Match
Charting Project's `charting-m-points-2020s.csv`, used to test the csv reader
and to run the parser against real data instead of just made-up strings.
source: jeff sackmann / tennis abstract, CC BY-NC-SA 4.0
(github.com/JeffSackmann/tennis_MatchChartingProject) - a small excerpt kept
here for tests, the full dataset isn't checked into this repo (see
`scripts/fetch_mcp_data.sh`).

still need: hand-decoded points covering plain rally, ace, double fault, second
serve, approach + net, lob + overhead, each error location, a 20+ shot rally.
