#!/bin/sh
# Run one BOF3 test unit through the shared gate.
# Usage: check-unit.sh <unit>
set -eu
ROOT="$(CDPATH='' cd -- "$(dirname "$0")/../../../.." && pwd)"
cd "$ROOT"
exec just check-unit "$@"
