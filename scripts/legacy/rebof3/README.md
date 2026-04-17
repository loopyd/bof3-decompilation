# scripts/rebof3

`scripts/rebof3/` is the repo-owned automation layer for the only supported decomp loop:

1. extract and inventory the disc
2. export one function from Ghidra
3. lift one function into `bof3/`
4. build it
5. diff it

Tracked incubating helpers live under `staging/`. Do not keep reusable tools in
plain `tmp/`.

## Canonical Commands

Prefer the root `Makefile` targets when they exist.

### Extraction and inventory

- `make extract`
- `make unpack`
- `make inventory`

### Ghidra export

- `make ghidra_decomp INPUT=... ADDR=...`
- `python3 -m scripts.rebof3 re ghidra-decomp <source> <address>`
- `python3 -m scripts.rebof3 re ghidra-decomp <source> <address> --asm-backend ghidra`
- `python3 -m scripts.rebof3 re ghidra-decomp <source> <address> --asm-backend spimdisasm`

This exports one per-function bundle under `tmp/ghidra_decomp/...` with:

- `func.json`
- `func.ghidra.c`
- `func.ghidra.s`
- `func.spim.s` when the alternative lane is enabled
- `func.s`
- `func.m2c.ctx.c`
- `func.m2c.ctx.i`
- optional `func.m2c.s`
- optional `func.m2c.c`

`func.s` is the canonical asm selected by `--asm-backend`. `func.m2c.s` is the
normalized PSX-aware asm feed for `m2c`. `func.m2c.ctx.c` and `func.m2c.ctx.i`
are the generated typed context inputs built from repo headers, curated PsyQ
headers, and source-mapped prototypes. `m2c` remains a side hint only. Ghidra
and the raw assembly remain the primary evidence.

### Matching

- `make match_init PROGRAM=... ENTRY=...`
- `make match_build PROGRAM=... ENTRY=...`
- `make match_diff PROGRAM=... ENTRY=... MATCH_DIFF_ARGS='--run-backend'`
- `make match_permuter PROGRAM=... ENTRY=... MATCH_PERMUTER_ARGS='--variant ghidra'`
- `make match_scaffold MATCH_SCAFFOLD_ARGS='--limit 50 --family ETC'`
- `make match_scaffold MATCH_SCAFFOLD_ARGS='--program-kind bin --asm-root tmp/asm-stage --exclude-glob "bof3/stubs/modules/boss*/*"'`
- `make match_sweep`
- `make match_compiler_report MATCH_COMPILER_REPORT_ARGS='--compiler-set tested-matrix'`

The default profile is `capcom97-bof3`:

- PsyQ 4.7
- ASPSX 2.56
- gcc 2.7.2-psx
- `maspsx`

Cross-compiler validation is optional. The canonical path is still
`gcc-2.7.2-psx`; extra old-gcc toolchains can be staged under
`deps/old_gcc_toolchains/` with:

- `make setup_old_gcc`
- `python3 -m scripts.rebof3 re setup-old-gcc --compiler gcc-2.8.0-psx`

## Source Mapping Rule

The canonical repo-owned naming style is address-stable:

- `func_80162d00`

The match tooling treats the entry address as the real identity. It accepts address-stable names such as `func_80162d00` or `FUN_80162d00`. New lifted code should not depend on `@source:` comments.

## Recommended Loop

For one function:

1. Confirm the address is a true function start in Ghidra.
2. Export the function bundle.
3. Lift it into the owning module in `bof3/`.
4. Keep names conservative.
5. Run `match_init`, `match_build`, and `match_diff`.
6. Fix semantics first.

When a function is close enough for automated exploration, `match_permuter` can
prepare and run `decomp-permuter` from the existing workspace. It defaults to
multithreaded execution with `-j <cpu_count / active_agents>` and can reuse the repo lift,
`func.ghidra.c`, or `func.m2c.c` as the starting source variant.
It now defaults to a bounded 60-second run inside the normal matching loop.
Its output goes to `permuter/permuter.log` by default; pass `--stdout` to stream
the live `decomp-permuter` output to the terminal instead.
Use `--variant m2c` when the assembly-to-`m2c` scratch path is a better seed
than the current repo lift.

## Secondary Commands

- `python3 -m scripts.rebof3 inventory ...`
- `python3 -m scripts.rebof3 re metadata ...`
- `python3 -m scripts.rebof3 match compiler-report ...`
- `python3 -m scripts.rebof3 match report ...`
- `python3 -m scripts.rebof3 match scaffold ...`
- `python3 -m scripts.rebof3 match scoreboard ...`
- `python3 -m scripts.rebof3 match frontier-backlog ...`
- `python3 -m scripts.rebof3 match import-backlog ...`
- `python3 -m scripts.rebof3 match import-wave ...`
- `python3 -m scripts.rebof3 match seed-wave ...`
- `python3 -m scripts.rebof3 match promote-wave ...`
- `python3 -m scripts.rebof3 match repair-wave ...`

`match compiler-report` supports:
- broad module runs with `--source-prefix`
- exact targeting with repeated `--source-file` or `--source-function`
- stdout tables/TSV/JSON with `--output-mode stdout` or `--output-mode both`
- structured report artifacts under the chosen report root with `--output-mode files` or `--output-mode both`

The wave/backlog commands are supported repo-owned automation, but they remain
secondary to the one-function lift/build/diff loop. Keep incubating helpers in
`staging/` until they reach that same stability bar.

`match scaffold` is the batch prep lane. It creates conservative disabled stubs
only where the repo can already justify them, then mirrors `func.s` and
`func.m2c.c` into `bof3/asm/...` using the same module/slot layout as the
corresponding source or stub path. Use `--asm-root` to stage that mirror into a
scratch tree first, and repeat `--exclude-glob` to skip known-problem path
classes in a broad batch.
