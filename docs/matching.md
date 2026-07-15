# Matching a function

> Compile one lifted function and compare it with the original PSX/MIPS code.

## Quick path

```sh
bin/harness targets "$TARGET"
bin/asmdiff "$FUNCTION_SOURCE"
```

An exact match exits with status `0`. A valid nonmatch exits with status `1`
and writes its evidence under `out/matching/`. Invocation, mapping, build, and
tool failures exit with status `2`.

Use `--json` when another local tool or agent consumes the result:

```sh
bin/asmdiff "$FUNCTION_SOURCE" --json
```

## Iteration order

1. Verify the payload and load address with `bin/harness targets "$TARGET"`.
2. Fix function boundaries and control flow before register allocation.
3. Check signedness, access width, constants, calls, and delay slots.
4. Use decompiler output only as a hint.
5. Once the source compiles and its boundary/control flow are credible, use a
   bounded permuter run when source shape or scheduling remains the issue:

```sh
bin/permute "$FUNCTION_SOURCE" --prepare-only
bin/permute "$FUNCTION_SOURCE" -j "$BOUNDED_JOBS"
bin/asmdiff "$FUNCTION_SOURCE"
```

Do not edit C to compensate for a constant address delta or incorrect target
mapping.

## Verification

```sh
just check
```

`just check` includes `bin/harness doctor --strict`. Run `just verify "$TARGET"`
only when claiming a whole-target byte match.
