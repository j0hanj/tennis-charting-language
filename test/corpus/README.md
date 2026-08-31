# Test corpus

Hand-decoded points used as golden tests for the lexer, parser, and sema.

Each case is a charting string plus the expected parsed structure (as JSON),
covering one feature: plain rally, ace, double fault, second serve, approach +
net, lob + overhead, forced vs unforced error, each error location, a 20+ shot
rally, and a few known-tricky real points.

A handful of full charted matches (with attribution) are kept here too, for
end-to-end score-reconstruction tests.

Source data: Match Charting Project, Jeff Sackmann / Tennis Abstract,
CC BY-NC-SA 4.0. See `../../NOTICE`.
