# Memory API (`include/base/` and `include/memory/`)

`include/base/` and `include/memory/` are the only API for fixed RAM
addresses, PS1 hardware registers, and scratchpad RAM from lifted C.
Fixed-address macros are casts; they access memory only when their result is
read or written. Put `const`/`volatile` directly on the `type` argument.

## Base types and compiler attributes (`include/base/`)

`include/base/types.h` defines `u8`..`u64`, `s8`..`s64`, `f32`, `f64`.
`include/base/compiler.h` owns the ordinary `NO_SIBLING_CALLS` compiler attribute
for preserving a real `jal` instead of a tail call. It replaces the former
`include/base/barrier.h`; it must not retain banned aids as no-op shims.

`REGISTER_PIN`, direct asm register bindings, `CLOBBER_*`, `barrier()` and all
artificial empty-asm matching barriers are banned under the
[source contract](matching.md#source-and-duplicate-rules), even after an exact
match or exhausted clean-C ladder. Handwritten inline assembly and
`extern X asm("NAME")` renames remain forbidden; `INCLUDE_ASM` needs separate
explicit approval. Bind fixed-address symbols with a plain
target-local `extern` in `internal.h` plus `WEAK_SYMBOL_AT(name, addr)` in
target `symbols.c`.

## Fixed addresses (`include/memory/access.h`)

- `PSX_PTR(type, address)` is a typed pointer to a fixed address.
  ```c
  PSX_PTR(u32, 0x80143B40u)
  PSX_PTR(volatile u16, 0x80143B90u)
  ```
- `PSX_REF(type, address)` is an lvalue at a fixed address.
  ```c
  PSX_REF(u32, 0x80143B40u) = value;
  value = PSX_REF(volatile u16, 0x80143B90u);
  ```
- `FIELD_ADDR(type, base, byte_offset)` / `FIELD_REF(type, base,
  byte_offset)` access an incomplete layout; replace with a reviewed struct
  member once known.
- `FUNCTION_AT(function_type, address)` calls a fixed-address function
  pointer; pass a function-pointer typedef as `function_type`.

Use `PSX_REF(volatile u8|u16|u32, address)` for a volatile hardware register
or unrecovered volatile fixed-RAM object; prefer a target-local symbol and
reviewed typed field once ownership and layout are known.

## Scratchpad (`include/memory/scratchpad.h`)

Scratchpad spans `0x1F800000`–`0x1F8003FF`.

- `SPAD_BASE` and `SPAD_SIZE` describe that range.
- `SPAD_ADDRESS(byte_offset)` gives an absolute byte address.
- `SPAD_ADDR(type, byte_offset)` is a typed pointer; `SPAD_REF(type,
  byte_offset)` is an lvalue.
- `SPAD_PTR_TABLE(type)` is a typed base for pointer cells; index by
  four-byte cell number when original code uses an offset `lw`, e.g.
  `SPAD_PTR_TABLE(Entity)[0x11]` for the cell at `0x1F800044`.
- `SPAD_PTR_SLOT(type, byte_offset)` is a non-volatile pointer-cell lvalue of
  type `type *`, intentionally matching the constant-address `lui + offset
  load` form of the original binary.

## Pointer-cell volatility

`volatile` on pointee vs pointer cell differs:

```c
PSX_REF(Entity *, addr)                    /* non-volatile cell */
PSX_REF(volatile Entity *, addr)           /* volatile pointee */
PSX_REF(Entity * volatile, addr)           /* volatile cell */
PSX_REF(volatile Entity * volatile, addr)  /* both */
```

`SPAD_PTR_SLOT(type, off)` keeps the cell non-volatile; qualifying `type`
cannot force a cell reload. Use explicit `PSX_REF(type * volatile,
SPAD_ADDRESS(off))` only when the original requires a reload per evaluation.
Validate with the owning function's live `asm-diff`/`byte-match`.
