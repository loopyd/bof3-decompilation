#include "bof3/bof3.h"

/* @source 0x801F3298
 * @behavior Overlay work-table release: zeroes byte 0x14 of each of the 0x10
 *           records of the 0x18-byte-stride table at 0x800E4800 (the main-RAM
 *           work area this overlay shares with the world and scenario
 *           overlays), walking the block with an unsigned byte counter in a
 *           bottom-tested loop. It takes no arguments, returns nothing and
 *           reads no state; the counter lives in $v1 and the masked compare
 *           result in $v0 only because the loop test needs them.
 *           Byte 0x14 is the record's active flag here: func_801F330C walks the
 *           same block from 0x800E4814 with stride 0x18, skips every record
 *           whose byte 0x14 reads zero (`lbu $v0, 0x0($s0)` at 0x801F3350,
 *           `beqz $v0` at 0x801F3358) and clears that same byte when the
 *           record's sub-phase countdown at +0x16 expires (0x801F33F8), so this
 *           function releases the whole block. Its only in-payload caller is
 *           func_801F2C6C (`jal` at 0x801F2C94): that step handler decrements
 *           work word 0xC, and when the count reaches zero it reloads the word
 *           with 0x230, calls this function and then advances the work object's
 *           byte at +0x01. The callee loads its own base and the caller ignores
 *           $v0, so the call site agrees with the no-argument void prototype.
 *           The frameless body and the `jr $ra` at 0x801F32BC prove the
 *           0x2C-byte boundary. Its 44 bytes have the same shape as the exact
 *           clearWorkTableByte15At800E4800World01Area05213_801F3194 lift
 *           (nonzero record offset cleared in place, stride 0x18, 0x10
 *           records); only the cleared offset differs here, so this target owns
 *           its own address and boundary.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearWorkTableByte14At800E4800World01Area04713_801F3298(void) {
  u8* work;
  u8 i;

  work = PSX_PTR(u8, 0x800E4800u);
  i = 0u;

  do {
    work[0x14] = 0;
    work += 0x18u;
    i += 1u;
  } while (i < 0x10u);
}
