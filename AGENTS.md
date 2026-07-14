# AGENTS.md — bof3-harness

BOF3 is a set of independently loaded binaries, not one link target. Read
[CONTEXT.md](CONTEXT.md) before naming or changing a binary, EMI entry, or
decompilation target.

## Workspace contract

- Work only in this repository; do not edit the sibling `rebof3/` checkout.
- `disks/` contains user-supplied US disc media. It is ignored and must never
  be committed.
- `out/` is the sole generated-artifact root. Do not create a legacy
  generated-artifact tree or reference one in source, configuration, commands,
  or documentation.
- `build/`, `out/`, and `toolchains/` are local/generated state. Never put
  authored source there.
- Durable binary-layout inputs live in `config/splat/` and `config/symbols/`.
  Reviewed analyzer replay and analysis-only type inputs live in
  `config/analysis/`; they do not override layouts or compiled types.
  Generated assembly, normalized images, catalogs, Ghidra state, diffs, and
  asset previews belong under `out/`.
- Before adding a path that might be ignored, run
  `git check-ignore -v <path>`. If it must be tracked, add the narrowest
  `.gitignore` exception and confirm it with `git status --short`.

## Repository layout

```text
asm/                  reviewed original assembly baselines
config/
  splat/              tracked binary layouts
  symbols/            tracked shared/authored symbols
  analysis/           tracked reviewed analyzer replay and type inputs
include/bof3/         shared C89 and PsyQ declarations
cmake/                build modules and source listings
src/
  exe/<binary>/       source for standalone PS-X executables
  emi/<family>/<archive>/<slot>/  source for confirmed EMI code targets
out/
  extracted/          disc tree and unpacked EMI entries
  binaries/           normalized raw executable images
  catalog/            raw EMI extraction evidence
  index/              repository-backed evidence graph and reports
  context/, lift/, matching/  retained decompilation evidence
  splat/, ghidra/, assets/  generated analysis products
  analysis/            generated analyzer projects and deterministic exports
```

Keep one `internal.h` beside each target's functions. Each lifted function is
one `func_XXXXXXXX.c` file. A large target may place focused declarations under
an adjacent `symbols/` directory. Keep the include path layered and singular:
`internal.h` includes `symbols/symbols.h`, which is the barrel for focused
`functions.h`, `variables.h`, and `files.h` declarations. Do not add a
target-local `psyq.h` when official PsyQ headers already declare the API.
Absolute bindings may be split into ordinary shallow `symbols/*.c` units;
retain `symbols.c` as the canonical target binding entry point. Do not use
`.inc` binding fragments or introduce competing declaration barrels.
For shared header guards, derive a short path-scoped name such as `CORE_EMI_H`;
do not repeat the repository name as a `BOF3_` prefix.

## Binary workflow

1. Place the user-owned disc image in `disks/`, then run `just setup`.
2. Inspect targets and generated evidence with `bin/harness target list` and
   `bin/harness index build`.
3. Promote only a reviewed code or mixed code/data EMI entry:

   ```sh
   bin/harness target promote "$ARCHIVE_ENTRY" --confirm-code
   ```

4. Work one function at a time:

   ```sh
   bin/harness lift <target> <function>
   # edit the printed C file
   bin/harness diff <source>
   ```

   After the lift compiles and its boundary and rough control flow are credible,
   run a bounded `bin/harness permute <source>` pass. Prefer this early for a
   same-size or >=80% candidate, then manually fix factual/type/control-flow
   issues and permute again when useful. Permuter output is not guaranteed to
   compile: require a successful base compile, real compiled candidates, and a
   fresh canonical `diff` before adopting it.

An EMI archive is a container; Splat and matching consume its extracted raw
entry, never the archive file. A type-0 entry is not automatically code.

Before declaring an executable address absent or zero-filled, read the PS-X EXE
header `t_addr` at offset `0x18` and compare it with the target manifest and
generated normalized-image metadata. `SLUS_004.22` loads at `0x80096800`, not
the common `0x80010000`; subtracting the wrong base maps real EMI-loader code
into unrelated zero padding. Original bytes and the PS-X header outrank target
metadata when they disagree; correct the tracked manifest before lifting.

## C and decompilation constraints

- Use C89 and readable, period-appropriate PSX C.
- Use `REG32()`, `REG16()`, and `REG8()` for hardware registers.
- Prefer named constants, declared externs, and recovered structs over magic
  addresses or values. Verify claims with Ghidra or disassembly before
  promoting them to shared declarations.
- PsyQ is library code: include/declare it as needed; do not lift or replace it.
- PsyQ addresses are target-local unless independently verified in each binary;
  record official SDK names and archive members, not one shared runtime address.
- Keep compiled function/data symbols address-based (`func_XXXXXXXX`,
  `DAT_XXXXXXXX`) so m2c, permuter, asm-diff, and analyzer replay remain
  traceable. Once meaning is proven, add a semantic alias in the owning
  `internal.h` or symbol layer and use it at readable call sites. Remove `LAB_`
  aliases once a callback or control-flow role is proven. `symbols.c` and its
  shallow `symbols/*.c` units own target-local address bindings; they are not a
  substitute for naming evidence.
- For an evidence-backed but unproven meaning, retain the address-based symbol
  and add a concise `INFERRED:` comment with the evidence and a verification
  path. Do not put hypotheses in `@behavior`, expose them as public contracts,
  or rename the symbol until the interpretation is reviewed.
- Do not use handwritten assembly to fill executable functions. Generated
  assembly stays under `out/splat/`.
- Preserve independent build targets: identical payload bytes at distinct load
  addresses are distinct until relocatability and symbol behaviour are proven.
- Lifted functions require a compact trace comment with `@behavior` and
  `@source`. Generated stubs use `@behavior Pending analysis`; replace that
  text with a factual description before promotion. The owning source path
  identifies the target, so do not add `@target`. Add at most one `@see` path
  when a tracked `docs/specs/` page provides material context. Never link
  generated state. Keep layouts and offset maps in the owning spec rather than
  duplicating them in C.

## Commands and verification

```sh
just extract       # extract disc content and unpack EMI archives
just build         # build promoted targets
just check         # tests, Ruff, and workspace doctor
just format        # all authored formatting
just format-python # Python tooling only
just format-c      # C headers and functions only
bin/harness doctor --strict
```

Run the smallest relevant command while iterating, then `just check` and
`bin/harness doctor --strict` before handoff when the required inputs/toolchain
are available. State any skipped check and why.

When a newly lifted function reaches a canonical 100% instruction/byte match,
re-run its diff and commit it immediately as a small focused change. Include
only the function and its required layout, declaration, or address binding;
exclude unrelated cleanup and generated evidence. Do not push unless explicitly
requested.

## Scope discipline

- Keep commits and external mutations out of scope unless explicitly asked.
- Do not hand-edit generated catalogs or binary output; regenerate them.
- Keep documentation factual and minimal: specs, verified findings, metadata,
  and durable learnings. Put generated tables and transient investigation notes
  in `out/`, not tracked docs.
- Add reusable, evidence-backed reverse-engineering gotchas and loop
  improvements to `LESSONS.md`. Keep transient hypotheses and raw command output
  under `out/`.
