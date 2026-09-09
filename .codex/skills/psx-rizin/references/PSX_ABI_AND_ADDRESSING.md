# PS1 MIPS ABI and address handling

## CPU model / endianness

Retail PSX: little-endian MIPS R3000A-family, MIPS I conventions + GTE coprocessor environment. Rizin raw code: MIPS, 32-bit, little-endian.

## General-purpose register roles

| register | conventional role | RE notes |
|---|---|---|
| `zero` | constant zero | writes discarded |
| `at` | assembler temporary | macros may use it unexpectedly |
| `v0`, `v1` | return values | also temporary inside leaf functions |
| `a0`–`a3` | first four argument words | call delay slot can still define one |
| `t0`–`t9` | caller-saved temporaries | not preserved across calls |
| `s0`–`s7` | callee-saved | often hold object/context pointers |
| `k0`, `k1` | kernel-reserved | BIOS/exception paths |
| `gp` | global/small-data pointer | may differ across modules/functions |
| `sp` | stack pointer | downward-growing stack |
| `fp`/`s8` | frame pointer or saved register | compiler-dependent |
| `ra` | return address | `jal` writes return PC; account for delay slot |

Extra arguments pass through the caller's stack argument area. Frame layout depends on compiler/version/optimization; infer from pre-call stores + callee loads.

## Delay slots

**Branch**: the instruction after a branch/jump executes before control transfers (subject to architectural exceptions). Treat it as part of branch/call semantics.

**Load**: the instruction after a load may observe the prior register value if it consumes the loaded register too soon. Compilers schedule around this; suspicious `nop` or independent instructions reveal generated-code patterns.

Call documentation minimum:

```text
four instructions before jal/jalr
jal/jalr
its delay-slot instruction
first instructions at target
```

## PS-X EXE header mapping

Conventional header is `0x800` bytes; little-endian fields:

| offset | field |
|---:|---|
| `0x10` | initial PC |
| `0x14` | initial GP |
| `0x18` | text load address |
| `0x1c` | text size |
| `0x20` | data load address |
| `0x24` | data size |
| `0x28` | BSS address |
| `0x2c` | BSS size |
| `0x30` | initial stack address/base |
| `0x34` | stack size/offset field |

Games don't always use every field conventionally. Validate header sizes against the actual file + startup code.

Normal text payload:

```text
payload_offset = file_offset - 0x800
runtime        = text_address + payload_offset
file_offset    = 0x800 + (runtime - text_address)
```

## RAM aliases

```text
physical_candidate = virtual_address & 0x1fffffff
cached_alias        = physical_candidate | 0x80000000
uncached_alias      = physical_candidate | 0xa0000000
```

Don't normalize arbitrary MMIO/BIOS addresses without checking the memory map. Preserve the original virtual address in traces — cache behavior can matter.

## Jump / call encodings

`j`/`jal` encode a 26-bit target combined with high bits of `PC + 4`:

```text
target = ((PC + 4) & 0xf0000000) | (imm26 << 2)
```

Scanner uses this only to identify candidates in a caller-supplied runtime range. Encoding match ≠ proof of code.

## GP-relative analysis

PsyQ/GCC code may access globals through `gp`. Determine GP from: PS-X EXE initial GP · startup/prologue init · module relocation/setup · runtime register captures · resolved-address consistency. Rizin `analysis.gp`/`analysis.gpfixed` are experimental per the handbook and GP may differ per function; prefer scoped/manual evidence over a forced global value.

## Signed offsets

MIPS load/store + `addiu` immediates are signed 16-bit. Record offsets in signed decimal AND hex — `0xffe0` is `-32`, not a large positive structure offset.
