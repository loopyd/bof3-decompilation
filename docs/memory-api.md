# Memory API (`include/base/` and `include/memory/`)

The canonical subsystem headers under `include/base/` and `include/memory/`
are the only API for fixed RAM addresses, PS1 hardware registers, and
scratchpad RAM from lifted C. Fixed-address macros are casts; they do not access
memory until their result is read or written. Put `const` and `volatile`
directly on the `type` argument.

## Base types and matching helpers (`include/base/`)

`include/base/types.h` defines `u8`..`u64`, `s8`..`s64`, `f32`, and `f64`.
`include/base/barrier.h` defines the only inline-assembly helpers allowed in
lifted code:

- `NO_SIBLING_CALLS` prevents a tail call when original code has a real `jal`.
- `barrier()` emits no instructions and only prevents compiler reordering.
  Use it for asm-diff-proven volatile-access ordering across a call.
- `CLOBBER_CALLER_REG(reg)` and named `CLOBBER_*` wrappers model an
  asm-diff-proven caller-clobbered register for delay-slot or fixed-address
  reload scheduling. They never select an opcode. Never clobber `s*`, `gp`,
  `sp`, or `ra`.
- `REGISTER_PIN(type, name, reg)` is a last-resort allocator constraint. After
  the clean-C ladder is exhausted, retain one only for an asm-diff-proven
  allocator or entry-register residual, with an adjacent `MATCHING_AID`,
  independent review, and a live byte match. It is not a scheduling tool.

Handwritten inline `__asm__`, direct `register X asm("$N")` pins,
`extern X asm("NAME")` renames, and `INCLUDE_ASM` are forbidden unless the
project rules explicitly authorize them. Bind fixed-address symbols with a
plain target-local `extern` in `internal.h` and `WEAK_SYMBOL_AT(name, addr)` in
the target `symbols.c`.

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
- `FIELD_ADDR(type, base, byte_offset)` and `FIELD_REF(type, base,
  byte_offset)` access an incomplete layout. Replace them with a reviewed
  struct member when the layout is known.
- `FUNCTION_AT(function_type, address)` calls a fixed-address function pointer.
  Pass a function-pointer typedef as `function_type`.

Use `PSX_REF(volatile u8|u16|u32, address)` for a volatile hardware register or
an unrecovered volatile fixed-RAM object. Prefer a target-local symbol and a
reviewed typed field when ownership and layout are known.

## Scratchpad (`include/memory/scratchpad.h`)

Scratchpad spans `0x1F800000` through `0x1F8003FF`.

- `SPAD_BASE` and `SPAD_SIZE` describe that range.
- `SPAD_ADDRESS(byte_offset)` gives an absolute byte address.
- `SPAD_ADDR(type, byte_offset)` is a typed pointer; `SPAD_REF(type,
  byte_offset)` is an lvalue.
- `SPAD_PTR_TABLE(type)` is a typed base for pointer cells. Index it by the
  four-byte cell number when original code uses an offset `lw`, such as
  `SPAD_PTR_TABLE(Entity)[0x11]` for the cell at `0x1F800044`.
- `SPAD_PTR_SLOT(type, byte_offset)` is a non-volatile pointer-cell lvalue of
  type `type *`. It intentionally matches the constant-address `lui + offset
  load` form used by the original binary.

## Pointer-cell volatility

`volatile` on the pointee and `volatile` on the pointer cell are different:

```c
PSX_REF(Entity *, addr)                    /* non-volatile cell */
PSX_REF(volatile Entity *, addr)           /* volatile pointee */
PSX_REF(Entity * volatile, addr)           /* volatile cell */
PSX_REF(volatile Entity * volatile, addr)  /* both */
```

`SPAD_PTR_SLOT(type, off)` deliberately keeps the cell non-volatile. It cannot
force a pointer-cell reload by qualifying `type`; use an explicit
`PSX_REF(type * volatile, SPAD_ADDRESS(off))` only when original assembly
requires a reload per evaluation. Validate the resulting address materialization
and load order with the owning function's live `asm-diff` and `byte-match`.
