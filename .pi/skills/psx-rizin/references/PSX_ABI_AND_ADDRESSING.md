# PS1 MIPS ABI and address handling

## CPU model and endianness

Retail PlayStation software runs on a little-endian MIPS R3000A-family CPU implementing MIPS I conventions plus the GTE coprocessor environment. For raw code in Rizin, use MIPS, 32-bit, little-endian settings.

## General-purpose register roles

| register | conventional role | reverse-engineering notes |
|---|---|---|
| `zero` | constant zero | writes are discarded |
| `at` | assembler temporary | macros may use it unexpectedly |
| `v0`, `v1` | return values | also temporary inside leaf functions |
| `a0`–`a3` | first four argument words | call delay slot can still define one |
| `t0`–`t9` | caller-saved temporaries | do not assume preservation across calls |
| `s0`–`s7` | callee-saved | often hold object/context pointers |
| `k0`, `k1` | kernel-reserved | BIOS/exception paths |
| `gp` | global/small-data pointer | may differ across modules/functions |
| `sp` | stack pointer | downward-growing stack |
| `fp`/`s8` | frame pointer or saved register | compiler-dependent |
| `ra` | return address | `jal` writes return PC; account for delay slot |

Extra arguments are normally passed through the caller’s stack argument area. The exact frame layout depends on compiler/version/optimization; infer it from stores before calls and loads in the callee.

## Delay slots

### Branch delay

The instruction immediately following a branch or jump executes before control transfers, subject to architectural exceptions. Treat it as part of the branch/call semantics.

### Load delay

On the original CPU, the instruction immediately after a load may observe the prior register value if it consumes the loaded register too soon. Compilers schedule around this, and suspicious `nop` or independent instructions can reveal generated-code patterns.

### Practical rule

When documenting a call, capture at least:

```text
four instructions before jal/jalr
jal/jalr
its delay-slot instruction
first instructions at target
```

## PS-X EXE header mapping

A conventional PS-X EXE has an `0x800`-byte header. Important little-endian fields used by the bundled parser:

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

Games do not always use every field conventionally. Validate header sizes against the actual file and startup code.

For a normal text payload:

```text
payload_offset = file_offset - 0x800
runtime        = text_address + payload_offset
file_offset    = 0x800 + (runtime - text_address)
```

## RAM aliases

The CPU exposes cached and uncached segments that can alias the same physical RAM. A useful comparison normalization for normal RAM addresses is:

```text
physical_candidate = virtual_address & 0x1fffffff
cached_alias        = physical_candidate | 0x80000000
uncached_alias      = physical_candidate | 0xa0000000
```

Do not normalize arbitrary MMIO/BIOS addresses without checking the memory map. Preserve the original virtual address in traces because cache behavior can matter.

## Jump and call encodings

MIPS `j`/`jal` encode a 26-bit target combined with the high bits of `PC + 4`. For a decoded instruction at runtime PC:

```text
target = ((PC + 4) & 0xf0000000) | (imm26 << 2)
```

The scanner uses this only to identify candidates in a caller-supplied runtime range. A matching encoding is not proof of code.

## GP-relative analysis

PsyQ/GCC-generated code may access globals through `gp`. Determine GP from:

1. PS-X EXE initial GP if used
2. startup/prologue initialization
3. module relocation/setup
4. runtime register captures
5. consistency of resolved addresses

Rizin exposes `analysis.gp` and `analysis.gpfixed`, but the official handbook describes this area as experimental and notes GP may differ per function. Prefer scoped/manual evidence over a forced global value.

## Signed offsets

MIPS load/store and `addiu` immediates are signed 16-bit values. Record offsets in both signed decimal and hexadecimal to avoid interpreting `0xffe0` as a large positive structure offset instead of `-32`.
