# Setup

> Set up the supported BOF3 reverse-engineering workspace.

## Contract

- Host: Linux x86_64.
- Game input: user-owned US BIN/CUE media under `disks/`; it is ignored by Git.
- Generated files: `build/`, `out/`, and `toolchains/`.
- Tracked binary layout: `config/splat/` and `config/symbols/`.

## Quick path

```bash
just psyq
just setup
bin/rebof3 doctor --strict
```

`just psyq` stages PsyQ 4.7 under `toolchains/psyq/4.7/`; it is required to
compile. SDK files remain ignored and must not be committed.

Use these day-to-day targets after setup:

```bash
just extract
just build
just check
just format
```

`just build` uses parallel Unix Makefiles. A restricted sandbox may block the
legacy PSX compiler; run the build outside that sandbox in that case.

## Local input

Keep the original US disc files in `disks/`. Do not commit game data, extracted
payloads, SDK files, or generated analysis output. See
[../disks/README.md](../disks/README.md) for the input boundary.

## Verification

After extraction, `bin/rebof3 scan` writes `out/catalog/emi.json`.
