# Matching a function

> Compile one lifted function and compare it with the original PSX/MIPS code.

## Quick path

```sh
bin/harness targets "$TARGET"
bin/m2c "$FUNCTION_SOURCE"       # automated C seed
# refine the seed into func_XXXXXXXX.c
bin/asmdiff "$FUNCTION_SOURCE"
```

The workflow is:

```text
Splat .s assembly
  -> bin/m2c (automated matching-oriented C seed)
  -> refine into authored C
  -> target compiler
  -> bin/asmdiff (acceptance)
```

bin/m2c produces a compilable C seed using m2c's matching-oriented
decompiler. Its output uses macros from `m2c_macros.h` for unknowns.
Refine the seed into the target source file, then validate with `bin/asmdiff`.

For focused manual search, run `--prepare-only`, edit the generated `base.c`
with upstream `PERM_*` directives, then run the same source with `--prepared` so
the wrapper does not overwrite those directives:

```sh
bin/permute "$FUNCTION_SOURCE" --prepare-only
# Edit out/permuter/<source-without-extension>/base.c.
bin/permute "$FUNCTION_SOURCE" --prepared -j "$BOUNDED_JOBS"
```

The prepared `base.c` contains the function and only the declarations and types
needed to compile it. It is generated from the real target compiler context.

An exact match exits with status `0`. A valid nonmatch exits with status `1`
and writes its evidence under `out/matching/`. Invocation, mapping, build, and
tool failures exit with status `2`.

Use `--json` when another local tool or agent consumes the result:

```sh
bin/asmdiff "$FUNCTION_SOURCE" --json
```

## Iteration order

1. Verify the payload and load address with `bin/harness targets "$TARGET"`.
2. Generate a C seed with `bin/m2c "$FUNCTION_SOURCE"`.
3. Fix function boundaries and control flow before register allocation.
4. Check signedness, access width, constants, calls, and delay slots.
5. Once the source compiles and its boundary/control flow are credible, use a
   bounded permuter run when source shape or scheduling remains the issue:

```sh
bin/permute "$FUNCTION_SOURCE" --prepare-only
bin/permute "$FUNCTION_SOURCE" -j "$BOUNDED_JOBS"
bin/asmdiff "$FUNCTION_SOURCE"
```

Run one permuter coordinator per function workspace. Its score ranks candidates;
only `bin/asmdiff` can accept a match.

Do not edit C to compensate for a constant address delta or incorrect target
mapping.

## Verification

```sh
just check
```

`just check` includes `bin/harness doctor --strict`. Run `just verify "$TARGET"`
only when claiming a whole-target byte match.
