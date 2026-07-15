# AGENTS.md — bof3-harness

BOF3 is a set of independently loaded binaries, not one link target. Read
[CONTEXT.md](CONTEXT.md) before naming or changing a binary, EMI entry, or
decompilation target. Load the narrowest workflow or domain skill needed; keep
always-on policy here.

## Workspace

- Work only in this repository; never edit the sibling `rebof3/` checkout.
- `inputs/` is ignored user media/private input and must never be committed.
- Authored inputs live under `src/`, `include/`, `Makefile`, `config/`, `asm/`,
  and `docs/`. `build/`, `out/`, and `toolchains/` are local/generated state.
- `out/` is the sole generated-artifact root. Do not invent a second output
  tree or hand-edit generated catalogs, projects, exports, or binaries.
- `config/splat/` and `config/symbols/` own durable layouts/symbols.
  `config/analysis/` owns reviewed replay and analysis-only types; it never
  overrides binary layouts or compiled declarations.
- Before adding a possibly ignored path, run `git check-ignore -v <path>` and
  use the narrowest exception if authored content must be tracked.

## Target and source ownership

- An executable, EMI archive, extracted entry, and promoted target are distinct
  objects. Analyze/build the exact executable payload or extracted entry, never
  an archive container. Preserve separate targets at separate load addresses.
- Original bytes and PS-X headers outrank metadata; reviewed tracked layouts
  outrank analyzer guesses. For PS-X executables, verify `t_addr` rather than
  assuming `0x80010000`.
- Keep one `func_XXXXXXXX.c` per lifted function and one adjacent `internal.h`.
  Large targets may use one singular barrel path:
  `internal.h -> symbols/symbols.h -> functions.h|variables.h|files.h`.
- Keep the same concise subsystem separators in each barrel header and its
  corresponding `symbols/*.c` binding unit, so declarations and absolute
  bindings remain navigable together.
- Order address-based `DAT_XXXXXXXX` declarations and bindings by ascending
  address within their owner section. Keep compatibility aliases alphabetical;
  order PsyQ SDK includes by their natural dependency order, then alphabetically
  when independent.
- Keep `symbols.c` as the target binding entry point; shallow `symbols/*.c`
  units are allowed. Do not use `.inc` binding fragments or target-local PsyQ
  headers when official SDK headers declare the API.
- A `symbols/psyq.h` barrel may record PsyQ binding ownership, but it only
  includes the official SDK wrapper; it never redeclares PsyQ APIs.
- Header guards are short and path-scoped, such as `CORE_EMI_H`; do not add a
  redundant repository prefix.

## Skills

The repository maintains five local skills under `.agents/skills/`. Load the
narrowest one before starting work; subagents do **not** inherit parent skills
and must either read the skill files themselves or receive skill content in
their prompt.

| Skill | Path | When to load |
| --- | --- | --- |
| `$bof3-docs` | `.agents/skills/bof3-docs/SKILL.md` | Locating the smallest authoritative repository document or command reference. |
| `$decomp-loop` | `.agents/skills/decomp-loop/SKILL.md` | Lifting, matching, or improving a PSX MIPS function in C89. Also read `references/matching-patterns.md` and `references/psx-mips-correctness.md`. |
| `$psx-rizin` | `.agents/skills/psx-rizin/SKILL.md` | Querying target-qualified analyzer evidence with direct Rizin, direct radare2, or the stateless harness adapter. Select adapter engines with `HARNESS_ANALYZER_ENGINE=rizin|r2|auto`. Also read `references/commands.md`, `references/psx-inputs.md`, and `references/projects-and-replay.md`. |
| `$bof3-specs` | `.agents/skills/bof3-specs/SKILL.md` | Interpreting payloads, EMI types, graphics, or cross-binary evidence. Also read `references/payload-map.md` and `references/evidence-promotion.md`. |
| `$psx1-hw` | `.agents/skills/psx1-hw/SKILL.md` | PSX1 hardware reference: memory map, registers, DMA, GPU, GTE, SPU, timers, CD-ROM, interrupts, and calling conventions. Read when touching hardware registers or MMIO. |

The harness command surface (`bin/harness`) is the primary workflow entry point
documented in `$decomp-loop`. Key commands:

```bash
bin/harness targets <target>
bin/harness reverse <target>@<8-digit-address> --run
bin/harness diff <source> --llm
bin/asmdiff <source>
bin/permute <source> -j <bounded-jobs>
```

`bin/permute` is the sole permuter entry point: one function, one generated
workspace, and one upstream decomp-permuter coordinator receiving the complete
`-j` worker count. Different functions may run independently, but never start
two coordinators for the same function workspace.

## Reverse-engineering invariants

- Use readable, period-appropriate C89 and the repository hardware-register
  helpers. Never fill executable functions with handwritten assembly.
- Keep compiled names address-based (`func_XXXXXXXX`, `DAT_XXXXXXXX`) for tool
  traceability. Add a semantic alias only after review; keep unproven meaning as
  a concise `INFERRED:` comment with evidence and a verification path.
- PsyQ code is external library code: use official declarations, prove identity
  from prototype plus assembly/call shape, and bind addresses target-locally.
  Do not lift PsyQ or assume addresses repeat across targets.
- Promoted functions require factual `@behavior` and address-preserving
  `@source`; use at most one material tracked `docs/specs/` `@see` link.
- Use `.agents/rules/` for detailed authored-C/build policy, `$bof3-specs` for
  payload interpretation, `$psx-rizin` for stateless analyzer evidence, and
  `$decomp-loop` for lifting, matching, permutation, and module completion.

## Verification and commits

- Run the narrowest build/diff while iterating. Before handoff run `just check`
  when available; it includes `bin/harness doctor --strict`. State skipped
  checks and reasons. Use `just verify <target>` only for a whole-target claim.
- When a newly lifted function reaches canonical 100% instruction and byte
  match, rerun its diff and prepare a focused change containing only the
  function plus required boundary, declaration, or binding.
- Do not stage, commit, push, or mutate external systems unless explicitly
  authorized.

## Knowledge hygiene

- Put stable reviewed findings in the owning compact `docs/specs/` concept and
  reusable evidence-backed gotchas in `LESSONS.md`. Keep raw output, generated
  tables, and transient hypotheses under `out/`.
- Preserve unrelated/user changes in a dirty worktree. Do not commit discs,
  generated artifacts, toolchains, or unrelated cleanup.
