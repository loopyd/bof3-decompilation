#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
ROOT_DIR="$(cd "${SCRIPT_DIR}/../.." && pwd)"
MAKE_BIN="${MAKE:-make}"
PSYQ_SOURCE="${PSYQ_SOURCE:-}"
PSYQ_ARCHIVE="${PSYQ_ARCHIVE:-}"
SKIP_PSYQ=0

usage() {
	cat <<'EOF'
Legacy wrapper retained during migration.

Usage:
  scripts/legacy/bin/setup_deps.sh [--psyq-source PATH] [--psyq-archive PATH] [--skip-psyq]
EOF
}

while [[ $# -gt 0 ]]; do
	case "$1" in
	--psyq-source)
		PSYQ_SOURCE="$2"
		shift 2
		;;
	--psyq-source=*)
		PSYQ_SOURCE="${1#--psyq-source=}"
		shift
		;;
	--psyq-archive)
		PSYQ_ARCHIVE="$2"
		shift 2
		;;
	--psyq-archive=*)
		PSYQ_ARCHIVE="${1#--psyq-archive=}"
		shift
		;;
	--skip-psyq)
		SKIP_PSYQ=1
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

if [[ ${SKIP_PSYQ} -eq 1 ]]; then
	exec "${MAKE_BIN}" -C "${ROOT_DIR}" setup_open
fi

exec "${MAKE_BIN}" -C "${ROOT_DIR}" setup \
	"PSYQ_SOURCE=${PSYQ_SOURCE}" \
	"PSYQ_ARCHIVE=${PSYQ_ARCHIVE}"
