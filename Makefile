PYTHON := $(CURDIR)/.venv/bin/python
UV := uv
UV_SYNC_FLAGS := --extra dev --frozen
PYTHONPATH := $(CURDIR)/tools/python
BIN_DIR := $(CURDIR)/bin
FORMAT_JOBS := $(shell nproc 2>/dev/null || getconf _NPROCESSORS_ONLN 2>/dev/null || echo 1)

.DEFAULT_GOAL := help

.PHONY: help venv doctor doctor-open setup-open setup pipeline extract inventory ghidra decomp-full-ready harness harness-ready lift-ready binary-parity configure build test fmt format format-c format-python

help:
	@printf '\n%s\n' 'Core'
	@printf '  %-16s %s\n' 'make venv' 'Sync the repo-local Python environment with uv'
	@printf '  %-16s %s\n' 'make doctor' 'Check the full workspace'
	@printf '  %-16s %s\n' 'make doctor-open' 'Check the fresh-clone open setup path'
	@printf '  %-16s %s\n' 'make setup-open' 'Run the open setup pipeline'
	@printf '  %-16s %s\n' 'make setup' 'Run the full setup pipeline'
	@printf '\n%s\n' 'Pipelines'
	@printf '  %-16s %s\n' 'make pipeline' 'List composable bin/pipeline recipes'
	@printf '  %-16s %s\n' 'make extract' 'Extract the disc and unpack EMI archives'
	@printf '  %-16s %s\n' 'make inventory' 'Refresh maintained inventory artifacts'
	@printf '  %-16s %s\n' 'make ghidra' 'Run the Ghidra bootstrap pipeline'
	@printf '  %-16s %s\n' 'make decomp-full-ready' 'Refresh extraction, Ghidra, symbol indexes, and harness state'
	@printf '\n%s\n' 'Harness'
	@printf '  %-16s %s\n' 'make harness' 'Show harness status and next action'
	@printf '  %-16s %s\n' 'make harness-ready' 'Refresh harness state, maps, reports, and dashboard'
	@printf '  %-16s %s\n' 'make lift-ready' 'Refresh cheap harness state for function lifting'
	@printf '  %-16s %s\n' 'make binary-parity' 'Build and diff compiled raw .bin files against originals'
	@printf '\n%s\n' 'Build'
	@printf '  %-16s %s\n' 'make configure' 'Configure the BOF3 PSX CMake preset'
	@printf '  %-16s %s\n' 'make build' 'Build the BOF3 PSX CMake preset'
	@printf '  %-16s %s\n' 'make test' 'Run Python tooling tests'
	@printf '  %-16s %s\n' 'make fmt' 'Format C/H and Python tooling'
	@printf '\n%s\n' 'Decomp'
	@printf '  %-16s %s\n' 'bin/asm-diff-one' 'Compile one source object and diff it against original asm'
	@printf '\n%s\n' 'Detailed tools live in ./bin.'

.venv/.ready: pyproject.toml uv.lock
	@$(MAKE) --no-print-directory venv

venv:
	@if ! command -v "$(UV)" >/dev/null 2>&1; then \
		printf 'missing uv; install uv and rerun `make venv`\n' >&2; \
		exit 1; \
	fi
	@"$(UV)" sync $(UV_SYNC_FLAGS)
	@touch "$(CURDIR)/.venv/.ready"

doctor: .venv/.ready
	@$(BIN_DIR)/doctor

doctor-open: .venv/.ready
	@$(BIN_DIR)/doctor-open

setup-open: .venv/.ready
	@$(BIN_DIR)/setup-open

setup: .venv/.ready
	@$(BIN_DIR)/setup

pipeline: .venv/.ready
	@$(BIN_DIR)/pipeline --list

extract: .venv/.ready
	@$(BIN_DIR)/disk-extract
	@$(BIN_DIR)/emi-unpack

inventory: .venv/.ready
	@$(BIN_DIR)/inventory-build

ghidra: .venv/.ready
	@$(BIN_DIR)/ghidra-bootstrap

decomp-full-ready: .venv/.ready
	@$(BIN_DIR)/pipeline decomp-full-ready

harness: .venv/.ready
	@$(BIN_DIR)/harness status
	@$(BIN_DIR)/harness resume

harness-ready: .venv/.ready
	@$(BIN_DIR)/pipeline harness-ready

lift-ready: .venv/.ready
	@$(BIN_DIR)/pipeline lift-ready

binary-parity: .venv/.ready
	@$(BIN_DIR)/pipeline binary-parity

configure:
	@$(BIN_DIR)/configure

build:
	@$(BIN_DIR)/build

test: venv
	@PYTHONPATH="$(PYTHONPATH)" $(PYTHON) -m pytest

fmt: format

format: format-c format-python

format-c:
	@find bof3/include bof3/src -type f \( -name '*.c' -o -name '*.h' \) -print0 | \
		xargs -0 -r -P "$(FORMAT_JOBS)" clang-format -i

format-python: venv
	@PYTHONPATH="$(PYTHONPATH)" $(PYTHON) -m ruff format tools/python
