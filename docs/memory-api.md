# Memory API (`include/` subsystem headers)

The subsystem headers under `include/` (`include/memory/`, `include/base/`,
etc.) are the single source of truth for reaching fixed RAM addresses, PS1
hardware registers, and scratchpad RAM from lifted C. The legacy paths under
`include/bof3/` remain as thin forwarding headers for backward compatibility.

All macros are plain casts; none perform a memory access on their own. Qualify
`const` and `volatile` by writing them directly on the `type` argument.

## `defines.h` (`include/base/types.h`, `include/base/barrier.h`)

Base integer types and the only two inline-assembly helpers allowed in lifted
code. Forwarding alias: `include/bof3/defines.h`.

- `u8`..`u64`, `s8`..`s64`, `f32`, `f64` — fixed-width scalar types.
- `NO_SIBLING_CALLS` — `__attribute__((optimize("no-optimize-sibling-calls")))`
  under GCC/Clang; empty otherwise. Use on a function when the original binary
  makes a real `jal` but the compiler tail-calls it.
- `barrier()` — `__asm__ __volatile__("" : : : "memory")`. Emits no
  instructions; only blocks compiler reordering across the barrier. Use when the
  problem is volatile-access **ordering** across a call (e.g. a global read must
  stay before an indirect call). See `LESSONS.md` for the `func_801F4578`
  example.
- `CLOBBER_CALLER_REG(reg)` — an empty-asm barrier that marks any named
  caller-clobbered MIPS register (`a0`–`a3`, `v0`/`v1`, or `t0`–`t9`)
  clobbered. The `CLOBBER_A0()` / `CLOBBER_A1()` / `CLOBBER_A2()` /
  `CLOBBER_V0()` / `CLOBBER_V1()` wrappers are preferred for those common
  registers. Use only after `asm-diff` identifies the register and placement
  needed to keep a C-generated instruction—`li`, `move`, arithmetic, load, or
  store—in a `jal`/branch **delay slot** or to retain a fixed-address reload
  ordering. It neither emits nor selects an opcode. Never clobber callee-saved
  `s*`, `gp`, `sp`, or `ra`; an `s*`
  allocator constraint remains an approved `REGISTER_PIN` case.

`barrier()`, `CLOBBER_*`, and `REGISTER_PIN(type, name, reg)` are
`__GNUC__`-guarded; the pin macro becomes a normal `register` declaration on
other compilers. `REGISTER_PIN` is a last-resort allocator constraint: after
the matching ladder is exhausted, make one bounded local experiment for an
asm-diff-proven allocator or entry-register residual with an adjacent
`MATCHING_AID` comment naming the
evidenced register, an independent review, and a live byte match. It is not a
generic scheduling tool. Use the compiler's supported allocator-register
spelling; a legacy numeric `"$N"` spelling still requires explicit user approval
and proof that the macro form does not preserve that function's codegen. The
pin's independent review is required before retention.

Inline `__asm__` is banned in lifted source except these macros. Direct
`register X asm("$N")` register pinning and `extern X asm("NAME")` symbol
renames are forbidden except for that verified legacy spelling; use
`REGISTER_PIN` for ordinary approved pins. Bind a fixed-address symbol
with a plain `extern` declaration in `internal.h` plus a `WEAK_SYMBOL_AT(name,
addr)` entry in the target `symbols.c` (`include/bof3/symbols.h`); never alias a
symbol with `__asm__`. A register pin is a last resort after the matching
ladder and may be tried autonomously only for an asm-diff-proven allocator or
entry-register residual; `INCLUDE_ASM` still requires user approval. First try
the per-object compiler-profile override in `config/compiler/object-flags.cmake`.

Rule of thumb: `barrier()` for access ordering, `CLOBBER_CALLER_REG(reg)`
(or a named wrapper) for evidenced caller-register scheduling,
`REGISTER_PIN` for an approved allocator constraint, and `WEAK_SYMBOL_AT` for
address binding.

## `memory.h` — address conversion (`include/memory/access.h`)

Forwarding alias: `include/bof3/memory.h`.

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

## `scratchpad.h` — 0x1F800000–0x1F8003FF (`include/memory/scratchpad.h`)

Forwarding alias: `include/bof3/scratchpad.h`.

- `SPAD_BASE` (`0x1F800000u`), `SPAD_SIZE` (`0x400u`).
- `SPAD_ADDRESS(byte_offset)` — absolute address of a scratchpad byte offset.
- `SPAD_ADDR(type, byte_offset)` — typed pointer into scratchpad; no access.
- `SPAD_REF(type, byte_offset)` — lvalue for an object stored in scratchpad.
- `SPAD_PTR_TABLE(type)` — typed base of scratchpad pointer cells. Index by
  four-byte cell number when the original uses `lui` + offset `lw`, e.g.
  `SPAD_PTR_TABLE(u8)[0x11]` for `0x1F800044`.
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

The repo adaptation of the matching-decomp foundation is
`docs/matching.md`.
