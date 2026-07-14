# PSX MIPS correctness checklist

Read this before promoting boundaries, control flow, types, or hardware effects.
Canonical bytes and the configured assembler/compiler remain authoritative.

## R3000A pipeline and integer behavior

- MIPS I branch/jump instructions execute one delay-slot instruction. Loads
  expose a load delay to the immediately following instruction on the PSX CPU.
- Integer arithmetic has no condition-code or carry flags. Recover comparisons,
  carry/borrow, overflow checks, and multiword arithmetic from explicit
  instructions.
- Multiply/divide write `HI`/`LO`; `mfhi`/`mflo` consume those asynchronous
  results. Preserve intervening scheduling and do not model them as ordinary C
  temporaries without checking emitted hazards.
- `lwl`/`lwr` and `swl`/`swr` are paired unaligned transfers. Recover byte order,
  effective addresses, and pair coverage before replacing them with a typed
  load/store or copy.
- Pseudo-instructions may expand. Compare real instructions/bytes, especially
  for `li`, `la`, `move`, relational branches, and large constants.
- `j`/`jal` encode a low 26-bit target field combined with the caller PC region;
  it is not a standalone absolute address.

## Addressing and memory

- GP-relative accesses may refer to compiler small-data placement, not a struct
  base or universal global table. Verify the target's `_gp`, section layout, and
  relocation context.
- Distinguish cached KSEG0, uncached KSEG1, scratchpad, and MMIO addresses.
  Pointer aliases can name the same physical RAM while differing operationally.
- Hardware-register accesses are volatile contracts. DMA setup requires channel
  registers, ordering, transfer mode, and completion/control evidence; a memory
  copy interpretation is insufficient.
- Cache-control and executable-code upload paths may require explicit cache
  behavior. Do not infer ordinary data semantics from a store alone.

## COP2/GTE

- COP2 transfers and GTE commands have instruction-specific register meanings,
  latency, saturation, and flag behavior. Use official/verified register and
  command definitions; do not flatten a GTE sequence into generic arithmetic
  solely from decompiler output.
- Check SDK macro expansion because period PsyQ macros can determine instruction
  order and register choice needed for an exact match.

## Sources

- [psx-spx CPU specifications](https://psx-spx.consoledev.net/cpuspecifications/)
- [psx-spx memory map](https://psx-spx.consoledev.net/memorymap/)
- [psx-spx DMA](https://psx-spx.consoledev.net/dmachannels/)
- [psx-spx GTE](https://psx-spx.consoledev.net/geometrytransformationenginegte/)
- [MIPS R3000 architecture manual](https://archive.org/details/bitsavers_mipsR3000R_6835608)
