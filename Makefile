SYSTEM_PYTHON ?= python3
PYTHON ?= $(CURDIR)/.venv/bin/python
PIP ?= $(CURDIR)/.venv/bin/pip
PYTHONPATH := $(CURDIR)/tooling
BIN_DIR := $(CURDIR)/bin
FORMAT_JOBS ?= $(shell nproc 2>/dev/null || getconf _NPROCESSORS_ONLN 2>/dev/null || echo 1)

SLUS ?= $(CURDIR)/build/extracted/SLUS_004.22
LOGO ?= $(CURDIR)/build/extracted/LOGO/LOGO.EXE
EMI_ROOT ?= $(CURDIR)/out/emi_raw/BIN

PSYQ_SOURCE ?=
PSYQ_ARCHIVE ?=
BOF3_ARCHIVE ?=

GHIDRA_BOOTSTRAP_DIR := $(CURDIR)/out/ghidra-bootstrap
INVENTORY_JSON := $(GHIDRA_BOOTSTRAP_DIR)/inventory.json
GROUPS_JSON := $(GHIDRA_BOOTSTRAP_DIR)/groups.json
GHIDRA_MANIFEST_JSON := $(GHIDRA_BOOTSTRAP_DIR)/ghidra_import_manifest.json
SETUP_PSYQ_FLAGS = $(if $(PSYQ_SOURCE),--psyq-source-root "$(PSYQ_SOURCE)") \
	$(if $(PSYQ_ARCHIVE),--psyq-archive "$(PSYQ_ARCHIVE)") \
	$(if $(BOF3_ARCHIVE),--disc-archive "$(BOF3_ARCHIVE)")

.DEFAULT_GOAL := help

.PHONY: help venv doctor doctor-open setup-plan setup-open-plan setup-open setup-submodules setup-private-assets setup-native-tools setup-psx-toolchain setup-match-tools setup setup-psyq setup-aspsx inventory configure build test test-python fmt format format-c format-python inventory-scan inventory-group inventory-build inventory-slot-map inventory-emi-catalog inventory-overlay-catalog inventory-overlay-clusters inventory-unique-overlay-map inventory-entry-tables inventory-project-plan inventory-render-metadata inventory-import-ghidra-symbols ghidra-plan ghidra-bootstrap ghidra-ui ghidra-install-extensions match-init match-build match-diff match-report disk-extract emi-unpack disk-rebuild emi-pack disk-verify disk-checksums

help:
	@printf '\n%s\n' 'Core'
	@printf '  %-24s %s\n' 'make venv' 'Create .venv and install the tooling package'
	@printf '  %-24s %s\n' 'make doctor' 'Check the full workspace, including local-only inputs'
	@printf '  %-24s %s\n' 'make doctor-open' 'Check the fresh-clone open setup path only'
	@printf '  %-24s %s\n' 'make setup-plan' 'Preview the full setup task sequence'
	@printf '  %-24s %s\n' 'make setup-open-plan' 'Preview the fresh-clone open setup sequence'
	@printf '  %-24s %s\n' 'make setup' 'Run the full workspace setup flow'
	@printf '\n%s\n' 'Inventory'
	@printf '  %-24s %s\n' 'make inventory' 'Refresh inventory, duplicate groups, and the Ghidra manifest'
	@printf '  %-24s %s\n' 'make inventory-scan' 'Scan extracted assets into inventory.json'
	@printf '  %-24s %s\n' 'make inventory-group' 'Group duplicate candidates into groups.json'
	@printf '  %-24s %s\n' 'make inventory-build' 'Build the maintained inventory artifact family'
	@printf '  %-24s %s\n' 'make inventory-overlay-catalog' 'Catalog overlay candidates from inventory.json'
	@printf '  %-24s %s\n' 'make inventory-entry-tables' 'Detect entry-table-shaped overlay payloads'
	@printf '  %-24s %s\n' 'make inventory-project-plan' 'Build import presets and entry-point labels'
	@printf '  %-24s %s\n' 'make inventory-import-ghidra-symbols' 'Reshape raw Ghidra export artifacts'
	@printf '  %-24s %s\n' 'make ghidra-plan' 'Build the Ghidra import manifest only'
	@printf '  %-24s %s\n' 'make ghidra-bootstrap' 'Run the full Ghidra bootstrap pipeline'
	@printf '  %-24s %s\n' 'make ghidra-ui' 'Launch the configured Ghidra UI'
	@printf '  %-24s %s\n' 'make ghidra-install-extensions' 'Install one or more Ghidra extensions'
	@printf '\n%s\n' 'Match'
	@printf '  %-24s %s\n' 'make match-init' 'Create one explicit function match workspace'
	@printf '  %-24s %s\n' 'make match-build' 'Run the configured workspace build command'
	@printf '  %-24s %s\n' 'make match-diff' 'Compare expected and actual workspace artifacts'
	@printf '  %-24s %s\n' 'make match-report' 'Aggregate workspace diff reports'
	@printf '\n%s\n' 'Setup'
	@printf '  %-24s %s\n' 'make setup-open' 'Run the full fresh-clone open setup path'
	@printf '  %-24s %s\n' 'make setup-submodules' 'Initialize git submodules only'
	@printf '  %-24s %s\n' 'make setup-private-assets' 'Initialize the optional private-assets workspace submodule'
	@printf '  %-24s %s\n' 'make setup-aspsx' 'Stage public ASPSX/PsyQ reference binaries'
	@printf '  %-24s %s\n' 'make setup-native-tools' 'Build bof3-disk and emi-ex only'
	@printf '  %-24s %s\n' 'make setup-psx-toolchain' 'Stage the canonical open PSX toolchain only'
	@printf '  %-24s %s\n' 'make setup-match-tools' 'Build objdiff-cli and mipsmatch only'
	@printf '  %-24s %s\n' 'make setup-psyq' 'Stage PsyQ 4.7 into toolchains/psyq/4.7'
	@printf '\n%s\n' 'Disk / EMI'
	@printf '  %-24s %s\n' 'make disk-extract' 'Extract the staged BOF3 disc image'
	@printf '  %-24s %s\n' 'make emi-unpack' 'Unpack all EMI archives from build/extracted/'
	@printf '  %-24s %s\n' 'make disk-rebuild' 'Rebuild a BOF3 disc image from the extracted project'
	@printf '  %-24s %s\n' 'make emi-pack' 'Pack unpacked EMI folders back into build/extracted/'
	@printf '  %-24s %s\n' 'make disk-verify' 'Verify staged disc images against disk checksums'
	@printf '  %-24s %s\n' 'make disk-checksums' 'Generate disk checksums for staged disc images'
	@printf '\n%s\n' 'Build'
	@printf '  %-24s %s\n' 'make configure' 'Configure the BOF3 PSX CMake preset'
	@printf '  %-24s %s\n' 'make build' 'Build the BOF3 PSX CMake preset'
	@printf '  %-24s %s\n' 'make test' 'Run Python tooling tests'
	@printf '\n%s\n' 'Style'
	@printf '  %-24s %s\n' 'make fmt' 'Format repo-owned BOF3 C/H and Python tooling'
	@printf '  %-24s %s\n' 'make format-c' 'Format BOF3 C/H sources with clang-format'
	@printf '  %-24s %s\n' 'make format-python' 'Format Python tooling with ruff'

.venv/.ready: pyproject.toml
	@$(SYSTEM_PYTHON) -m venv "$(CURDIR)/.venv"
	@$(PIP) install -e '.[dev]'
	@touch "$@"

venv: .venv/.ready

doctor: .venv/.ready
	@$(BIN_DIR)/doctor

doctor-open: .venv/.ready
	@$(BIN_DIR)/doctor-open

setup-plan: .venv/.ready
	@$(BIN_DIR)/setup-plan

setup-open-plan: .venv/.ready
	@$(BIN_DIR)/setup-open-plan

configure:
	@$(BIN_DIR)/configure

build:
	@$(BIN_DIR)/build

test: test-python

fmt: format

format: format-c format-python

inventory: ghidra-bootstrap

test-python: .venv/.ready
	@PYTHONPATH="$(PYTHONPATH)" $(PYTHON) -m pytest

format-c:
	@find bof3/include bof3/src -type f \( -name '*.c' -o -name '*.h' \) -print0 | \
		xargs -0 -r -P "$(FORMAT_JOBS)" clang-format -i

format-python: .venv/.ready
	@PYTHONPATH="$(PYTHONPATH)" $(PYTHON) -m ruff format tools/python

inventory-scan: .venv/.ready
	@$(BIN_DIR)/inventory-scan \
		--slus "$(SLUS)" \
		--logo "$(LOGO)" \
		--emi-root "$(EMI_ROOT)" \
		--output "$(INVENTORY_JSON)"

inventory-group: .venv/.ready
	@$(BIN_DIR)/inventory-group \
		--input "$(INVENTORY_JSON)" \
		--output "$(GROUPS_JSON)"

inventory-build: .venv/.ready
	@$(BIN_DIR)/inventory-build \
		--slus "$(SLUS)" \
		--logo "$(LOGO)" \
		--emi-root "$(EMI_ROOT)"

inventory-slot-map: .venv/.ready
	@$(BIN_DIR)/inventory-slot-map \
		--slus "$(SLUS)"

inventory-emi-catalog: .venv/.ready
	@$(BIN_DIR)/inventory-emi-catalog \
		--emi-root "$(EMI_ROOT)"

inventory-overlay-catalog: .venv/.ready
	@$(BIN_DIR)/inventory-overlay-catalog \
		--inventory "$(INVENTORY_JSON)" \
		--groups "$(GROUPS_JSON)"

inventory-overlay-clusters: .venv/.ready
	@$(BIN_DIR)/inventory-overlay-clusters

inventory-unique-overlay-map: .venv/.ready
	@$(BIN_DIR)/inventory-unique-overlay-map

inventory-entry-tables: .venv/.ready
	@$(BIN_DIR)/inventory-entry-tables

inventory-project-plan: .venv/.ready
	@$(BIN_DIR)/inventory-project-plan

inventory-render-metadata: .venv/.ready
	@$(BIN_DIR)/inventory-render-metadata

inventory-import-ghidra-symbols: .venv/.ready
	@$(BIN_DIR)/inventory-import-ghidra-symbols

ghidra-plan: .venv/.ready
	@$(BIN_DIR)/ghidra-plan \
		--inventory "$(INVENTORY_JSON)" \
		--groups "$(GROUPS_JSON)" \
		--output "$(GHIDRA_MANIFEST_JSON)"

ghidra-bootstrap: .venv/.ready
	@$(BIN_DIR)/ghidra-bootstrap \
		--slus "$(SLUS)" \
		--logo "$(LOGO)" \
		--emi-root "$(EMI_ROOT)" \
		--output-dir "$(GHIDRA_BOOTSTRAP_DIR)"

ghidra-ui: .venv/.ready
	@$(BIN_DIR)/ghidra-ui

ghidra-install-extensions: .venv/.ready
	@$(BIN_DIR)/ghidra-install-extensions

match-init: .venv/.ready
	@$(BIN_DIR)/match-init

match-build: .venv/.ready
	@$(BIN_DIR)/match-build

match-diff: .venv/.ready
	@$(BIN_DIR)/match-diff

match-report: .venv/.ready
	@$(BIN_DIR)/match-report

setup-psyq: .venv/.ready
	@$(BIN_DIR)/setup-psyq \
		$(SETUP_PSYQ_FLAGS)

setup-aspsx: .venv/.ready
	@$(BIN_DIR)/setup-aspsx

setup-submodules: .venv/.ready
	@$(BIN_DIR)/setup-submodules

setup-private-assets: .venv/.ready
	@$(BIN_DIR)/setup-private-assets

setup-native-tools: .venv/.ready
	@$(BIN_DIR)/setup-native-tools

setup-psx-toolchain: .venv/.ready
	@$(BIN_DIR)/setup-psx-toolchain

setup-match-tools: .venv/.ready
	@$(BIN_DIR)/setup-match-tools

setup-open: .venv/.ready
	@$(BIN_DIR)/setup-open

setup: .venv/.ready
	@$(BIN_DIR)/setup \
		$(SETUP_PSYQ_FLAGS)

disk-extract: .venv/.ready
	@$(BIN_DIR)/disk-extract

emi-unpack: .venv/.ready
	@$(BIN_DIR)/emi-unpack

disk-rebuild: .venv/.ready
	@$(BIN_DIR)/disk-rebuild

emi-pack: .venv/.ready
	@$(BIN_DIR)/emi-pack

disk-verify: .venv/.ready
	@$(BIN_DIR)/disk-verify

disk-checksums: .venv/.ready
	@$(BIN_DIR)/disk-checksums
