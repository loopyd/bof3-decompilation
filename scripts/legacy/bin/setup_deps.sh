#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
ROOT_DIR="$(cd "${SCRIPT_DIR}/../.." && pwd)"
MAKE_BIN="${MAKE:-make}"
PSYQ40_SOURCE="${PSYQ40_SOURCE:-}"
PSYQ40_ARCHIVE="${PSYQ40_ARCHIVE:-}"
SKIP_PSYQ40=0

usage() {
	cat <<'EOF'
Legacy wrapper retained during migration.

Usage:
  scripts/legacy/bin/setup_deps.sh [--psyq40-source PATH] [--psyq40-archive PATH] [--skip-psyq40]
EOF
}

while [[ $# -gt 0 ]]; do
	case "$1" in
	--psyq40-source)
		PSYQ40_SOURCE="$2"
		shift 2
		;;
	--psyq40-source=*)
		PSYQ40_SOURCE="${1#--psyq40-source=}"
		shift
		;;
	--psyq40-archive)
		PSYQ40_ARCHIVE="$2"
		shift 2
		;;
	--psyq40-archive=*)
		PSYQ40_ARCHIVE="${1#--psyq40-archive=}"
		shift
		;;
	--skip-psyq40)
		SKIP_PSYQ40=1
		shift
		;;
	-h | --help)
		usage
		exit 0
		;;
	*)
		printf 'unknown option: %s\n' "$1" >&2
		usage >&2
		exit 1
		;;
	esac
done

if [[ ${SKIP_PSYQ40} -eq 1 ]]; then
	exec "${MAKE_BIN}" -C "${ROOT_DIR}" setup_open
fi

exec "${MAKE_BIN}" -C "${ROOT_DIR}" setup \
	"PSYQ40_SOURCE=${PSYQ40_SOURCE}" \
	"PSYQ40_ARCHIVE=${PSYQ40_ARCHIVE}"
