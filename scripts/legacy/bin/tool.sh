#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(CDPATH='' cd -- "$(dirname -- "$0")" && pwd)"
WORKSPACE="$(CDPATH='' cd -- "${SCRIPT_DIR}/../.." && pwd)"

usage() {
	cat <<'EOF'
Legacy helper retained during migration.

Use `scripts/_bof3 ...`, `make ...`, or direct tool invocations instead.
EOF
}

case "${1:-help}" in
help|-h|--help)
	usage
	;;
*)
	printf 'legacy wrapper moved: use scripts/_bof3 or make targets instead\n' >&2
	exit 1
	;;
esac
