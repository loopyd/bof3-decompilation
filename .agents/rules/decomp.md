# Coding Rules
- C89: `/* */` comments, declare vars at top of function
- `REG32()`, `REG16()`, `REG8()` from `bof3/include/bof3/defines.h` for hardware register access
- `DEFINE_FUNC_AT()` in `bof3/include/bof3/context.h` only for not-yet-lifted cross-module calls; remove when lifted
- One function per `.c` file, self-contained with `#include "internal.h"`
- No new headers in `bof3/include/bof3/modules/` — ALL declarations go in module's `internal.h`
- Readable clean C > forcing 100% match when too complex
- No inline assembly; prefer defines/structs/externs over magic addresses
