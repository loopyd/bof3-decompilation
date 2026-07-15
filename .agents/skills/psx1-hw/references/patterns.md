# PSX1 Reverse Engineering Patterns

Common code patterns found in PSX MIPS binaries. Recognize these to lift
functions faster and avoid miscompilation.

## Address loading

### 32-bit absolute address (most common)

```asm
lui   $at, %hi(D_XXXXXXXX)
lw    $reg, %lo(D_XXXXXXXX)($at)
```

Two-instruction sequence: `$at` gets upper 16 bits, load adds signed lower
16 bits. In C: just reference the symbol directly.

### GP-relative data (PsyQ common)

```asm
lw    $reg, %gp_lo(D_XXXXXXXX)($gp)
```

Requires `$gp` to be set up. Used for data near the global pointer.

### Scratchpad access

```asm
lui   $at, 0x1F80
lw    $reg, 0x0044($at)
```

Direct access to scratchpad at `0x1F800000`. BOF3 stores a per-overlay
pointer at `0x1F800044`.

## Branch patterns

### Simple conditional

```asm
beq   $a0, $zero, label
nop                         # delay slot (often nop)
```

### Branch with useful delay slot

```asm
beq   $a0, $zero, label
addiu $v0, $zero, 1         # executes before branch taken
```

### Branch-likely (delay slot only executes if taken)

```asm
beql  $a0, $zero, label
addiu $v0, $zero, 1         # only executes if branch taken
```

### Compare and branch

```asm
slt   $at, $a0, $a1        # $at = ($a0 < $a1) ? 1 : 0
bne   $at, $zero, label     # branch if $a0 < $a1
```

### Unsigned compare

```asm
sltu  $at, $a0, $a1
bne   $at, $zero, label
```

## Loop patterns

### Counted loop (down)

```asm
li    $t0, 8               # loop count
loop:
  # ... body ...
  addiu $t0, $t0, -1
  bne   $t0, $zero, loop
  nop
```

### Counted loop (up, compare)

```asm
li    $t0, 0               # index
li    $t1, 8               # limit
loop:
  # ... body ...
  addiu $t0, $t0, 1
  slt   $at, $t0, $t1
  bne   $at, $zero, loop
  nop
```

### Pointer loop

```asm
la    $t0, array_start
la    $t1, array_end
loop:
  # ... body ...
  addiu $t0, $t0, 4         # advance pointer
  slt   $at, $t0, $t1
  bne   $at, $zero, loop
  nop
```

## Function calls

### Standard call

```asm
jal   func_XXXXXXXX
nop                         # delay slot (often nop)
```

### Tail call (return after call)

```asm
j     func_XXXXXXXX         # no jal, no $ra save
nop
```

### Indirect call (function pointer)

```asm
jalr  $t9                   # call through register
nop
```

### Register preservation

```asm
# Prologue: save $ra and $s0-$sN
addiu $sp, $sp, -N
sw    $ra, N-4($sp)
sw    $s0, N-8($sp)
# ... function body ...
# Epilogue: restore
lw    $s0, N-8($sp)
lw    $ra, N-4($sp)
jr    $ra
addiu $sp, $sp, N           # delay slot restores stack
```

## Return patterns

### Standard return

```asm
jr    $ra
nop                         # delay slot
```

### Return with value

```asm
jr    $ra
addiu $v0, $a0, 0           # move $a0 to $v0 in delay slot
```

### Return with stack restore in delay slot

```asm
lw    $ra, 0x1c($sp)
lw    $s0, 0x18($sp)
jr    $ra
addiu $sp, $sp, 0x20        # delay slot: stack restore
```

## Switch/jump table patterns

### Jump table (direct)

```asm
sll   $t0, $a0, 2           # index * 4
la    $t1, jtbl_XXXXXXXX
addu  $t0, $t1, $t0         # table + offset
lw    $t0, 0($t0)           # load target address
jr    $t0
nop
```

### Jump table (offset from table base)

```asm
sll   $t0, $a0, 2
la    $t1, jtbl_XXXXXXXX
addu  $t0, $t1, $t0
lw    $t0, 0($t0)
addu  $t0, $t0, $t1         # base + offset
jr    $t0
nop
```

### If-else chain (small switch)

```asm
beq   $a0, $zero, case_0
nop
li    $at, 1
beq   $a0, $at, case_1
nop
li    $at, 2
beq   $a0, $at, case_2
nop
# default case
```

## Multiply/divide patterns

### Multiply (32-bit)

```asm
mult  $a0, $a1
mflo  $v0                    # result in LO
```

### Multiply (unsigned)

```asm
multu $a0, $a1
mflo  $v0
```

### Divide

```asm
div   $zero, $a0, $a1       # quotient in LO, remainder in HI
mflo  $v0                    # quotient
mfhi  $v1                    # remainder
```

### Divide by constant (optimized)

```asm
# Divide by 16: shift right 4
sra   $v0, $a0, 4
# Divide by 256: shift right 8
sra   $v0, $a0, 8
```

### Modulo (power of 2)

```asm
# a0 % 16: mask lower 4 bits
andi  $v0, $a0, 0xF
```

## Bit manipulation

### Extract bit field

```asm
srl   $v0, $a0, 8           # shift right
andi  $v0, $v0, 0xFF        # mask 8-bit field
```

### Set bit

```asm
ori   $a0, $a0, 0x8000      # set bit 15
```

### Clear bit

```asm
li    $at, ~0x8000
and   $a0, $a0, $at          # clear bit 15
```

### Test bit

```asm
andi  $at, $a0, 0x8000
bne   $at, $zero, bit_set
```

## Memory copy patterns

### Word copy (aligned)

```asm
la    $t0, src
la    $t1, dst
li    $t2, count             # word count
copy:
  lw    $t3, 0($t0)
  sw    $t3, 0($t1)
  addiu $t0, $t0, 4
  addiu $t1, $t1, 4
  addiu $t2, $t2, -1
  bne   $t2, $zero, copy
  nop
```

### Byte copy

```asm
copy:
  lbu   $t3, 0($t0)
  sb    $t3, 0($t1)
  addiu $t0, $t0, 1
  addiu $t1, $t1, 1
  bne   $t2, $zero, copy
  addiu $t2, $t2, -1
```

## Register allocation hints

| Pattern | Likely meaning |
| --- | --- |
| `$s0–$s7` saved across calls | Local variables |
| `$t0–$t9` not saved | Temporaries |
| `$a0–$a3` at function entry | Arguments |
| `$v0` before `jr $ra` | Return value |
| `$gp` in function body | Global data access |
| `$fp` as frame pointer | Structured stack frame |
| `$sp` manipulation | Stack frame |

## Signed vs unsigned

| Instruction | Signedness |
| --- | --- |
| `addu`/`addiu` | Unsigned (no overflow trap) |
| `add`/`addi` | Signed (traps on overflow) |
| `subu` | Unsigned subtract |
| `mult` | Signed multiply |
| `multu` | Unsigned multiply |
| `div` | Signed divide |
| `divu` | Unsigned divide |
| `slt` | Signed compare |
| `sltu` | Unsigned compare |
| `lbu` | Unsigned byte load |
| `lb` | Signed byte load (sign-extends) |
| `lhu` | Unsigned halfword load |
| `lh` | Signed halfword load |

PsyQ GCC 2.7.2 defaults to `addu`/`subu` for arithmetic (no overflow traps).
Use `sra` (shift right arithmetic) for signed division by power of 2.

## Common BOF3-specific patterns

### Overlay loading

```asm
# Load overlay header, jump to entry
lw    $t0, 0($a0)           # overlay entry point
jalr  $t0
nop
```

### Scratchpad work area

```asm
lui   $at, 0x1F80
lw    $t0, 0x0044($at)      # SCRATCH_PTR
# use $t0 as base for scratchpad data
```

### DMA GPU ordered list

```asm
# Channel 2: GPU linked list
lui   $at, 0x1F80
li    $t0, DMA_START | DMA_SYNC_LINKED
sw    $a0, 0x10A0($at)      # D_MADR = list address
sw    $zero, 0x10A4($at)    # D_BCR = 0
sw    $t0, 0x10A8($at)      # D_CHCR = start linked list
```

### VSync wait

```asm
# Wait for VBlank interrupt
wait_vblank:
  lw    $t0, 0($gp)         # VSync counter
  beq   $t0, $zero, wait_vblank
  nop
```

## Delay slot gotchas

- Instructions after `j`/`jal`/`jr`/`beq`/`bne` always execute
- The compiler fills delay slots with useful instructions when possible
- Don't reorder instructions across branch boundaries
- `bin/asmdiff` compares the full instruction stream including delay slots
- A `nop` in a delay slot means the compiler couldn't find anything useful

## Common decompilation mistakes

1. Forgetting delay slots — the instruction after a branch always executes
2. Treating `addu` as signed — it's unsigned, use context
3. Missing `$gp` relative accesses — check for `%gp_lo`/`%gp_hi`
4. Wrong multiply result — `mult` puts 64-bit result in `HI:LO`
5. Ignoring `jr $ra` delay slot — it often contains a useful instruction
6. Treating `sltu` as signed comparison
7. Missing jump table base adjustment
8. Confusing `lb` (sign-extends) with `lbu` (zero-extends)
