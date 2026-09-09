#!/bin/sh
set -eu
ROOT="$(CDPATH='' cd -- "$(dirname "$0")/../../../.." && pwd)"
exec "$ROOT/bin/naming-evidence-run" "$@"
