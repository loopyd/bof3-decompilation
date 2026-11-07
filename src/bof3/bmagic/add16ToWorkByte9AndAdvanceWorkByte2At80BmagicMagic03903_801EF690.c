#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell, already bound at 0x1F800044 by this
 * target's map. The symbolic address lets the assembler fold it into each load
 * (lui $vN,0x1F80 / lw $vN,68($vN)), so the original's two separate reads each
 * re-materialise the base: one load shared by the byte-9 read-modify-write and
 * a second load after the store through the first pointer. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EF690
 * @behavior Per-frame ramp step of the overlay work object. Reads the
 *           scratchpad work-object pointer cell at 0x1F800044 and adds 0x10 to
 *           the object's byte at offset 0x9 with a byte-width read-modify-write
 *           (lbu / addiu 0x10 / sb). The value just stored is zero-extended
 *           with andi 0xFF - no second load - and compared against 0x80, which
 *           is materialised with addiu $v0,$zero,0x80. Only when that byte has
 *           ramped to 0x80 does it re-read the pointer cell and increment the
 *           dispatch state byte at offset 0x2 (lbu / addiu 1 / sb), which
 *           advances this overlay's per-frame work byte 2 handler sequence. Leaf
 *           with no frame; takes no arguments, returns nothing and reads or
 *           writes no other state.
 * @status exact
 * @match 100.00
 * @residual none
 */
void add16ToWorkByte9AndAdvanceWorkByte2At80BmagicMagic03903_801EF690(void) {
  if ((g_battle_work[9] += 0x10) == 0x80) {
    g_battle_work[2]++;
  }
}
