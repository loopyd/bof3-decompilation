# Coding Rules
- Format authored C with the root `.clang-format`: compact K&R braces, two-space indentation, 80 columns, and left-bound pointers.
- Derive header guards from the shortest unambiguous authored path, for example
  `CORE_EMI_H` or `EMI_GAME_01_INTERNAL_H`. Do not add a redundant
  repository-wide `BOF3_` prefix.
- C89: `/* */` comments, declare vars at top of function
- `REG32()`, `REG16()`, `REG8()` from `include/bof3/defines.h` for hardware register access
- Keep function declarations as ordinary `extern` declarations; place any
  original-address fallback in the calling target's `symbols.c` or shallow
  `symbols/*.c` units with `WEAK_SYMBOL_AT()`.
- No new headers in `include/bof3/modules/` — ALL declarations go in module's `internal.h`
- Readable clean C > forcing 100% match when too complex
- Large targets use one layered declaration path: `internal.h` includes
  `symbols/symbols.h`, which includes focused `functions.h`, `variables.h`, and
  `files.h` headers. Do not add `psyq.h` when official PsyQ headers suffice.
- No inline assembly in function bodies. `WEAK_SYMBOL_AT()` is the sole
  assembly helper and is allowed only in target-owned `symbols.c` and shallow
  `symbols/*.c` files. Use ordinary C units, never `.inc` binding fragments.
- Fixed-address RAM globals: declare `extern type DAT_xxxxx;` in `internal.h`
  with a `/* @behavior ... */` comment, and place them with
  `WEAK_SYMBOL_AT(DAT_xxxxx, 0x8XXXXXXX)` in the owning executable's symbol
  binding units; never use `#define DAT_xxxxx VUxx(addr)`.
- `DAT_xxx` defines live in `internal.h`; readable semantic name → `DAT_xxx` mappings live in `symbols.h` once promoted
- Before promotion, keep an address-based `func_XXXXXXXX`/`DAT_XXXXXXXX` name.
  When evidence supports a useful hint but does not prove the meaning, add one
  concise `INFERRED:` comment beside the owning declaration with the observed
  evidence and the check needed to promote it. Do not encode the hint as a
  semantic alias or in an `@behavior` trace field.
- After promotion, keep the compiled address-based identifier and expose the
  verified semantic name as a simple alias in the owning `internal.h`/symbol
  layer. Keep the source filename and `@source` address unchanged. Prefer
  semantic typedefs and recovered structs; they improve readability without
  obscuring binary provenance.

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
