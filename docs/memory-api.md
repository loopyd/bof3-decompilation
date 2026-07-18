# Memory API (`include/bof3/`)

The headers under `include/bof3/` are the single source of truth for reaching
fixed RAM addresses, PS1 hardware registers, and scratchpad RAM from lifted C.
They replaced the older `PTR_AT`/`OBJECT_AT`/`MMIO8-32`/`SPAD_PTR`/`SPAD_OBJECT`
macros and the `vu8`/`vu16`/`vu32` typedefs.

All macros are plain casts; none perform a memory access on their own. Qualify
`const` and `volatile` by writing them directly on the `type` argument.

## `defines.h`

Base integer types and the only two inline-assembly helpers allowed in lifted
code.

- `u8`..`u64`, `s8`..`s64`, `f32`, `f64` — fixed-width scalar types.
- `NO_SIBLING_CALLS` — `__attribute__((optimize("no-optimize-sibling-calls")))`
  under GCC/Clang; empty otherwise. Use on a function when the original binary
  makes a real `jal` but the compiler tail-calls it.
- `barrier()` — `__asm__ __volatile__("" : : : "memory")`. Emits no
  instructions; only blocks compiler reordering across the barrier. Use when the
  problem is volatile-access **ordering** across a call (e.g. a global read must
  stay before an indirect call). See `LESSONS.md` for the `func_801F4578`
  example.
- `CLOBBER_A0()` / `CLOBBER_V0()` / `CLOBBER_A1()` — empty-asm barriers that
  mark the named MIPS register clobbered. Use when the problem is **which
  register** holds a value in a `jal`/branch **delay slot** (e.g. keep
  `move a0,zero` in the slot before `func_801C1400(0u)`).

Both `barrier()` and the `CLOBBER_*` macros are `__GNUC__`-guarded; they expand
to nothing on other compilers. Never write raw `__asm__` in a `.c` file.

Rule of thumb: `barrier()` for access ordering, `CLOBBER_A0/V0/A1()` for
delay-slot placement.

## `memory.h` — address conversion

- `PSX_PTR(type, address)` — typed pointer to a fixed address.
  ```c
  PSX_PTR(u32, 0x80143B40u)
  PSX_PTR(volatile u16, 0x80143B90u)
  PSX_PTR(const u8, 0x80010000u)
  ```
- `PSX_REF(type, address)` — lvalue for an object stored at a fixed address.
  ```c
  PSX_REF(u32, 0x80143B40u) = value;
  value = PSX_REF(volatile u16, 0x80143B90u);
  ```
- `FIELD_ADDR(type, base, byte_offset)` / `FIELD_REF(type, base, byte_offset)` —
  byte-offset access into an incomplete struct; replace with real struct members
  once the layout is understood.
  ```c
  value = FIELD_REF(u32, g_game_work, 0x18u);
  ```
- `FUNCTION_AT(function_type, address)` — fixed-address function pointer. Always
  pass a function-pointer typedef as `function_type`.
  ```c
  typedef void (*Handler)(void);
  FUNCTION_AT(Handler, 0x80123456u)();
  ```
- `REG8/16/32(address)` — PS1 memory-mapped **hardware registers only**. These
  are `volatile`; never use them for scratchpad RAM.

## `scratchpad.h` — 0x1F800000–0x1F8003FF

- `SPAD_BASE` (`0x1F800000u`), `SPAD_SIZE` (`0x400u`).
- `SPAD_ADDRESS(byte_offset)` — absolute address of a scratchpad byte offset.
- `SPAD_ADDR(type, byte_offset)` — typed pointer into scratchpad; no access.
- `SPAD_REF(type, byte_offset)` — lvalue for an object stored in scratchpad.
- `SPAD_PTR_SLOT(type, byte_offset)` — a **pointer cell** stored in scratchpad:
  `PSX_REF(type *, SPAD_ADDRESS(byte_offset))`. The cell is intentionally
  **not** `volatile`. The constant-address codegen (`lui` + offset `lw`/`lh`/
  `lb`, e.g. `lw v1,68(v1)` for `0x1F800044`) matches the original binary.
  Marking the cell `volatile` forces `lui + ori + lw`, which diverges
  (`func_801B5BDC` regressed from 100% to a mismatch when the slot was made
  volatile). To force a reload per evaluation, qualify the pointee
  (`SPAD_PTR_SLOT(volatile Entity, off)`) rather than the cell.

## Pointer-cell volatility

- `PSX_REF(type *, addr)` — non-volatile cell; loaded once, reused.
- `PSX_REF(type * volatile, addr)` — volatile cell; reloaded on every evaluation.
- `SPAD_PTR_SLOT(type, off)` is the non-volatile form by design.

## Migration map

| Old                       | New                                  |
| ------------------------- | ------------------------------------ |
| `vu8`/`vu16`/`vu32`       | `volatile u8`/`u16`/`u32`            |
| `PTR_AT(type, addr)`      | `PSX_PTR(type, addr)`                |
| `OBJECT_AT(type, addr)`   | `PSX_REF(type, addr)`                |
| `MMIO8/16/32(addr)`       | `REG8/16/32(addr)`                   |
| `SPAD_PTR(type, off)`     | `SPAD_ADDR(type, off)`               |
| `SPAD_OBJECT(type, off)`  | `SPAD_REF(type, off)`                |
| `PTR_SLOT_AT(type, addr)` | `PSX_REF(type *, addr)`              |
