#!/usr/bin/env bash
# Fetch the Match Charting Project dataset into ./data (git-ignored).
#
# The data is licensed CC BY-NC-SA 4.0 by Jeff Sackmann / Tennis Abstract and is
# NOT redistributed in this repository. This script clones it locally so you can
# run tcl against the full corpus. See NOTICE.

set -euo pipefail

REPO="https://github.com/JeffSackmann/tennis_MatchChartingProject.git"
DEST="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)/data/mcp"

if [ -d "$DEST/.git" ]; then
  echo "Updating existing clone in $DEST"
  git -C "$DEST" pull --ff-only
else
  echo "Cloning Match Charting Project into $DEST"
  mkdir -p "$(dirname "$DEST")"
  git clone --depth 1 "$REPO" "$DEST"
fi

echo "Done. CSV files are under $DEST"
