#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(CDPATH='' cd -- "$(dirname -- "$0")" && pwd)"
WORKSPACE="$(CDPATH='' cd -- "${SCRIPT_DIR}/../.." && pwd)"

cat <<EOF
This wrapper was moved under scripts/legacy/bin during the rebof3-simple cleanup.
If you still need it, port it back intentionally against the new workflow.
Workspace: ${WORKSPACE}
EOF
