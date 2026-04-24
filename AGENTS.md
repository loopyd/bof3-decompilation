# AGENTS.md - rebof3-simple

## Scope

- Work inside `rebof3-simple/`.
- Do not edit the sibling `rebof3/` tree unless the user explicitly asks.
- Use `bin/` as the maintained command surface.
- Keep `make` limited to setup, extraction, inventory, Ghidra bootstrap, build,
  test, and formatting.

## Setup Checks

Use these commands to confirm the repo is ready:

```bash
make doctor
make extract
make inventory
make ghidra
make configure
make build
bin/doctor --strict
```

## Decomp Loop

Work one function at a time:

```bash
bin/asm-diff-one bof3/src/core/emi/func_80162178.c
```

Read the generated outputs under `out/asm-diff/<function>/`, edit the matching
source/header, then rerun the same command.

Prefer visible source or header definitions over linker-script symbol patches.
Use `bof3/include/bof3/original_symbols.h` only for not-yet-lifted original
calls and remove entries as functions are lifted.

## Verification

Before handing work back, run the smallest relevant checks plus:

```bash
bin/build
bin/doctor --strict
PYTHONDONTWRITEBYTECODE=1 .venv/bin/python -m pytest -q -p no:cacheprovider tools/python/tests
```
