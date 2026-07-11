# Coding Rules
- Format authored C with the root `.clang-format`: compact K&R braces, two-space indentation, 80 columns, and left-bound pointers.
- C89: `/* */` comments, declare vars at top of function
- `REG32()`, `REG16()`, `REG8()` from `include/bof3/defines.h` for hardware register access
- `DEFINE_FUNC_AT()` in `include/bof3/context.h` only for not-yet-lifted cross-module calls; remove when lifted
- No new headers in `include/bof3/modules/` — ALL declarations go in module's `internal.h`
- Readable clean C > forcing 100% match when too complex
- No inline assembly; prefer defines/structs/externs over magic addresses
- Fixed-address RAM globals: declare `extern type DAT_xxxxx;` in `internal.h`
  with a `/* @behavior ... */` comment, and place them with
  `SYMBOL_AT(DAT_xxxxx, 0x8XXXXXXX)` in `src/boot/symbols.c`; never use
  `#define DAT_xxxxx VUxx(addr)`.
- `DAT_xxx` defines live in `internal.h`; readable semantic name → `DAT_xxx` mappings live in `symbols.h` once promoted

## Trace comments

Promoted functions keep one compact comment immediately above the definition:

```c
/* @behavior Tests one flag bit in the entry state.
 * @source 0xXXXXXXXX original_label
 * @see docs/specs/runtime/example.md
 */
```

- `@behavior` states observable behavior. Generated stubs use
  `@behavior Pending analysis`; replace it before promotion.
- `@source` is required and records the runtime address plus original label.
- Do not add `@target`; the owning source path identifies the binary.
- `@see` is optional. Add at most one tracked `docs/specs/` path when it
  provides material context that should not be duplicated in C.
- Do not link generated state, investigation output, or another C file.
- Describe observable behavior; do not narrate instructions or register movement.
- Do not add confidence or uncertainty markers to source comments. Keep
  unresolved findings in generated analysis until the evidence is sufficient.
- Put layouts, offsets, and cross-function mappings in the owning OKF spec and
  reference its relative path instead of copying a long explanation into C.
- Remove speculative names and stale `possible name` comments when promoting.
