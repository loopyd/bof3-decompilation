# .agents — BOF3 project agent config

## Rules (always loaded)

- `rules/decomp.md` — C89 decompilation conventions (REG32, DAT_xxx, internal.h)
- `rules/build.md` — build verification targets (asm-diff-one, make, pytest)

## Skills (on-demand via `skill` tool)

- `decomp-loop` — one-function lift-and-match loop (quick CLI + harness + tool chain)
- `harness` — bin/harness workflow (target selection, claiming, lifecycle, reports)
- `bof3-docs` — doc index for project docs and specs
