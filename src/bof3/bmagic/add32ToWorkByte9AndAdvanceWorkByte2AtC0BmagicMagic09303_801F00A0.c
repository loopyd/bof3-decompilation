#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell, already bound at 0x1F800044 by this
 * target's map. The symbolic address lets the assembler fold it into each load
 * (lui $vN,0x1F80 / lw $vN,68($vN)), so the original's two separate reads each
 * re-materialise the base: one load shared by the byte-9 read-modify-write and
 * a second load after the store through the first pointer. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801F00A0
 * @behavior Per-frame ramp step of the overlay work object. Reads the
 *           scratchpad work-object pointer cell at 0x1F800044 once and adds
 *           0x20 to the object's byte at offset 0x9 with a byte-width
 *           read-modify-write (lbu / addiu 0x20 / sb). The value just stored is
 *           zero-extended with andi 0xFF - no second load - and compared
 *           against 0xC0, which is materialised with addiu $v0,$zero,0xC0. Only
 *           when that byte has ramped to 0xC0 does it re-read the pointer cell
 *           and increment the dispatch state byte at offset 0x2 (lbu / addiu 1
 *           / sb), which advances this overlay's per-frame work byte 2 handler
 *           sequence selected through the dispatchStateByte2 table. Leaf with
 *           no frame; returns nothing and reads or writes no other state.
 * @status exact
 * @match 100.00
 * @residual none
 */
void add32ToWorkByte9AndAdvanceWorkByte2AtC0BmagicMagic09303_801F00A0(void) {
  if ((g_battle_work[9] += 0x20) == 0xC0) {
    g_battle_work[2]++;
  }
}
