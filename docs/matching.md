# Matching a function

> Compile one lifted function and compare it with the original PSX/MIPS code.

## Quick path

```sh
bin/rebof3 inspect "$TARGET"
bin/rebof3 diff "$FUNCTION_SOURCE"
```

An exact match exits with status `0`. A valid nonmatch exits with status `1`
and writes its evidence under `out/asm-diff/`. Invocation, mapping, build, and
tool failures exit with status `2`.

Use `--json` when another local tool or agent consumes the result:

```sh
bin/rebof3 diff "$FUNCTION_SOURCE" --json
```

## Iteration order

1. Verify the payload and load address with `inspect`.
2. Fix function boundaries and control flow before register allocation.
3. Check signedness, access width, constants, calls, and delay slots.
4. Use decompiler output only as a hint.
5. Use decomp-permuter only after the function compiles and is structurally
   close.

Do not edit C to compensate for a constant address delta or incorrect target
mapping.

## Verification

```sh
just build
just check
bin/rebof3 doctor --strict
```
