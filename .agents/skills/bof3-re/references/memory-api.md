# Memory API

`include/base/` and `include/memory/` are sole fixed-RAM/hardware/scratchpad API;
no raw address casts. Macros cast only; access occurs on read/write. Qualify `type`
directly. Prefer reviewed target-local symbol/field once ownership/layout known.

`include/base/types.h`: `u8` through `u64`, `s8` through `s64`, `f32`, `f64`.
`include/base/compiler.h`: ordinary `NO_SIBLING_CALLS` preserves real `jal`.
Register pins/clobbers/empty-asm barriers/no-op substitutes stay banned. Plain
local extern plus manifest-owned `WEAK_SYMBOL_AT` binds address; no asm-renamed
extern. `INCLUDE_ASM` requires separate explicit approval.

| API | Meaning |
| --- | --- |
| `PSX_PTR(type,address)` | Typed fixed-address pointer |
| `PSX_REF(type,address)` | Fixed-address lvalue |
| `FIELD_ADDR(type,base,byte_offset)` / `FIELD_REF(...)` | Incomplete layout access; replace with reviewed member when known |
| `FUNCTION_AT(function_type,address)` | Function pointer; typedef must be function-pointer type |
| `SPAD_BASE`, `SPAD_SIZE` | Scratchpad `0x1F800000` through `0x1F8003FF` |
| `SPAD_ADDRESS(byte_offset)` | Absolute scratchpad byte address |
| `SPAD_ADDR(type,byte_offset)` / `SPAD_REF(...)` | Typed pointer/lvalue |
| `SPAD_PTR_TABLE(type)` | Pointer-cell base, four-byte indices |
| `SPAD_PTR_SLOT(type,byte_offset)` | Non-volatile `type *` cell lvalue, constant-address offset-load form |

```c
PSX_REF(u32, 0x80143B40u) = value;
value = PSX_REF(volatile u16, 0x80143B90u);
SPAD_PTR_TABLE(Entity)[0x11]; /* cell at 0x1F800044 */
PSX_REF(Entity *, addr);                    /* plain cell */
PSX_REF(volatile Entity *, addr);           /* volatile pointee */
PSX_REF(Entity * volatile, addr);           /* volatile cell */
PSX_REF(volatile Entity * volatile, addr);  /* both */
```

Qualifying `SPAD_PTR_SLOT` type cannot force cell reload. Use
`PSX_REF(Entity * volatile, SPAD_ADDRESS(off))` only with original per-evaluation
reload evidence. Volatility needs asynchronous/hardware-mutation evidence, not
matching preference. Live owning asm-diff/byte-match required.
