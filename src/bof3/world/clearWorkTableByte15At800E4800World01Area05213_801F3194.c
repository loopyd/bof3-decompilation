#include "bof3/bof3.h"

/* @source 0x801F3194
 * @behavior Overlay work-table clear: it zeroes byte 0x15 of each of the 0x10
 *           records of the 0x18-byte-stride table at 0x800E4800 (the main-RAM
 *           work area this overlay shares with the world and scenario
 *           overlays), walking the block with an unsigned byte counter in a
 *           bottom-tested loop. It takes no arguments, returns nothing and
 *           reads no state; the counter is left in $v0/$v1 only because the
 *           loop test (masked to 0xFF, compared below 0x10) needs it. Byte
 *           0x15 is the record's occupancy flag in this payload: func_801F3158
 *           returns the first record of the same 0x10-entry/0x18-byte-stride
 *           block whose byte 0x15 is zero (`lbu $v0, 0x15($v1)` at 0x801F3164,
 *           `beqz $v0` at 0x801F316C), and func_801F31C0 walks the same block
 *           from 0x800E4815 and skips every record whose flag is zero
 *           (`lbu $v0, 0x0($s0)` at 0x801F31F8 with $s0 = 0x800E4815), so this
 *           function releases the whole block. Its only in-payload caller is
 *           the func_801F2D08 row (`jal` at 0x801F2D90): the call arrives
 *           after that path has cleared byte 0x6 of the struct the 0x1F800044
 *           cell points at, its delay slot clears byte 0x9 of the same struct,
 *           and the row continues with `jal 0x8015DF18(0x203)`. The callee
 *           loads $a0 itself and the caller ignores $v0, so the call site
 *           agrees with the no-argument void prototype. The prologue-less body
 *           and the `jr $ra` at 0x801F31B8 prove the 0x2C-byte boundary. Its
 *           44 bytes have the same shape as the exact
 *           clearWorkTableByte25At800E4800ScenarioScena0400_801F7FCC lift
 *           (nonzero record offset cleared in place); the count, stride and
 *           cleared offset differ here, so this target owns its own address and
 *           boundary.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearWorkTableByte15At800E4800World01Area05213_801F3194(void) {
  u8* work;
  u8 i;

  work = PSX_PTR(u8, 0x800E4800u);
  i = 0u;

  do {
    work[0x15] = 0;
    work += 0x18u;
    i += 1u;
  } while (i < 0x10u);
}
