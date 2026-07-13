# Reverse-engineering lessons

Durable findings that make the BOF3 lift-and-match loop faster and safer.

## Verify boundaries before lifting

- Splat labels are reviewed inputs, not stronger evidence than payload bytes.
- A plausible decoded instruction can still be embedded data. In
  `emi/world00/area008/13`, bytes at payload offset `0x14` begin with the
  `"%d"` entry header; executable code starts at offset `0x18`.
- Check that calls, saved return addresses, and return paths are coherent. A
  false start at `0x801f2c14` appeared to call before its prologue and would
  have returned into itself.
- Split confirmed leading data explicitly in Splat before promoting the real
  function boundary.

## Treat analyzer bases carefully

- Raw EMI payloads may contain a header before the configured code VRAM.
- Reconcile analyzer addresses with payload offsets and the Splat segment
  start before promoting a boundary. The canonical payload bytes and tracked
  layout remain authoritative over analyzer-created function names.
- Do not load an extracted `GAME.EMI` entry as one linear raw image at its
  first function address. Entry 0 begins with a count/pointer header and entry
  1 loads at `0x801d0c00`, begins with a control word, and does not reach the
  title setup handler until payload offset `0x90` (`0x801d0c90`); using that
  handler as the target load address shifts every payload offset. Normalize and
  split the entry through the harness, then verify boundaries against canonical
  lift assembly.
- `GAME.EMI#0` loads at `0x80195800`; its first reviewed function is at payload
  offset `0x91c` (`0x8019611c`). Configuring the target at the first function
  silently shifts direct byte reads and analyzer addresses by `0x91c`.
  Sequence-based asm resolution can still report plausible or exact function
  matches under that bad base, so a green function diff does not validate the
  target load address. Cross-check the catalog/header destination and require
  `runtime address - load address == payload offset` before adding boundaries.
- Frontend callback tables at `0x801c7b08` and `0x801c7b14` are zero in the
  shipped SLUS load image and populated at runtime. Recover their consumers and
  producers from code/xrefs; do not treat the static zero-filled EXE bytes as
  evidence that the callbacks are absent.

## Cross-check executable metadata

- Read PS-X EXE `t_addr` from header offset `0x18`; do not assume the common
  `0x80010000` base. `SLUS_004.22` loads at `0x80096800`.
- A wrong target-manifest base can map valid runtime addresses into unrelated
  zero padding. The EMI loader at `0x80161f58` is present at normalized-image
  offset `0xcb758`; subtracting the former `0x80010000` manifest base produced
  the false offset `0x151f58` and an apparent all-zero library.
- Cross-check the tracked target manifest against the normalized binary's
  generated metadata and the original PS-X EXE header before concluding that
  code is runtime-generated or missing.
- Apply the same check independently to every PS-X executable. `LOGO.EXE`
  loads at `0x801ce000`; treating it as a common-base `0x80010000` image puts
  its real entry point and reviewed functions outside the normalized payload.

## Diagnose tool failures outside the candidate

- If `bin/harness diff` cannot compile a new lift, compile one known existing
  source from the same target. The same failure on both sources indicates a
  workspace or toolchain problem, not evidence that the candidate C is wrong.
- A compiler exit without diagnostics is not a comparison result. Preserve the
  last verified diff and fix the compile path before tuning source shape.
- The historical compiler is a statically linked 32-bit i386 executable. Under
  a managed sandbox it can exit `225` before processing arguments, even for
  `--version`. Re-run the repository `bin/cc` driver with its approved
  out-of-sandbox permission; do not add flags or rewrite C to address `225`.
- Build historical PsyQ objects serially when validating a large target. A
  parallel archive build can reach `ar` while a compiler output is temporarily
  absent, then leave that same object present after the failure. Re-run with
  `cmake --build build/default --target <target> -j1` before diagnosing the C.

## Do not synthesize an executable link model

- A partial set of lifted SLUS objects is a validation archive, not a rebuilt
  PS-X executable. Do not invent a CRT entry point, linker layout, or probe loop
  to make it link.
- `LOGO.EXE` is independently loaded. A SLUS helper that copies its streaming
  loop and calls LOGO-local addresses crosses the binary ownership boundary;
  preserve such investigation evidence only outside the compiled SLUS source
  set.

## Isolate equivalence-test output

- Extractor parity tests must create a unique directory under `/tmp` and remove
  only that exact directory. Never use repository `out/` as scratch output or a
  cleanup root: it contains the user's extracted media and all retained local
  reverse-engineering evidence.
- Resolve and validate the temporary path before cleanup, install a scoped trap,
  and reject empty, root, repository, or repository-`out` cleanup targets.

## Keep symbol ownership target-local

- PsyQ library code can be linked more than once at different addresses across
  executables and EMI payloads. An address verified in `SLUS_004.22` is not a
  shared address contract for another binary.
- Use official PsyQ function names and record the verified archive member in the
  owning target manifest. Put any runtime address fallback in that target's
  `symbols.c`.
- Replace analyzer `LAB_` aliases once behavior and signature are proven, but
  retain the compiled `func_XXXXXXXX`/`DAT_XXXXXXXX` identifier. Expose the
  verified meaning through a simple semantic alias so tools and humans can both
  trace the original address.
- Preserve useful pre-promotion evidence with an `INFERRED:` comment beside the
  owning address-based declaration. State what was observed and what would
  verify promotion; do not create a semantic alias from a hint alone.

## Record the hard tail precisely

- Report size, matching instruction count, percentage, and first mismatch.
- A commutative operand-order mismatch can be ABI-equivalent but still prevent
  an exact byte match. Keep readable C unless a bounded source permutation
  produces the original instruction without false types or undefined behavior.

## Use matching tools throughout convergence

- Use m2c with the generated function context as a reconstruction hint or to
  cross-check control flow, calls, and access widths; canonical assembly remains
  authoritative.
- Use bounded decomp-permuter runs whenever they may produce a better source
  shape, including early searches for the correct instruction count and size.
  Prioritize it at `90%+`, where remaining differences are more likely expression
  shape, declaration order, register allocation, or scheduling.
- Retain a permuter result only when it preserves factual, readable C89. Exact
  bytes remain authoritative; semantic-equivalence reporting must not weaken the
  exact-match criterion.
- Generated permuter settings must name `bin/objdump`; the repository wrapper
  resolves the staged MIPS objdump even when it is absent from the user's PATH.
- A permuter `target.o` copied from the current C object creates a false
  score-zero self-comparison. Build the target from authoritative original
  function bytes before trusting any candidate score.
- The permuter compile script is called with candidate input/output arguments;
  a generated script must honor those paths rather than always compiling its
  seed `base.c` to a fixed object.
- Resolve relative candidate and output arguments before changing into CMake's
  compile directory. Otherwise the same script succeeds manually with absolute
  paths but every real permuter candidate fails to compile.
- Permuter input must be self-contained enough for its `cpp -nostdinc` pass.
  Blindly preprocessing the full PsyQ header tree can retain legacy parameter
  names such as `$30`, which the permuter parser rejects.
- Validate the bundle with a real bounded run, not preparation alone. A working
  loader bundle reached a stable nonzero base score and compiled more than 100
  candidates against an original-byte-derived target object.
