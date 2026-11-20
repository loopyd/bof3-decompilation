#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell shared by the battle and bmagic
 * overlays. This target's own map row g_battle_work binds 0x1F800044, so the
 * cell keeps that name here; its symbolic address folds into each load
 * (lui $vN,0x1F80 / lw $vN,0x44($vN)), and the original re-reads the cell for
 * the byte-0x2 group because the byte store that precedes it goes through the
 * pointer the cell supplied. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EF850
 * @behavior Per-step handler of the overlay's work-object state machine. It
 *   reads the scratchpad work-object pointer cell at 0x1F800044 once (lui
 *   $v0,0x1F80 / lw $v0,0x44($v0), one empty load-delay slot), increments that
 *   object's unsigned byte 0xA by one at byte width (lbu $v1,0xA($v0) /
 *   addiu $v1,$v1,1 / sb $v1,0xA($v0)) and tests the truncated value it just
 *   stored (andi $v1,$v1,0xFF, with no second load of the cell) against 0x16
 *   for equality (addiu $v0,$zero,0x16 / bne with an empty delay slot). Only
 *   when that byte equals 0x16 does it re-materialize the cell with its own
 *   lui/lw pair and increment the object's byte 0x2 at byte width (lbu
 *   $v0,0x2($v1) / addiu $v0,$v0,1 / sb $v0,0x2($v1)). The 80-byte body has
 *   no frame, no call and no other store: jr $ra at 0x801EF89C with an empty
 *   delay slot, and no jal in this payload reaches 0x801EF850, whose only
 *   in-image reference is the pointer word at 0x801F0374 - word 2 of the
 *   pointer run workByte2HandlerTableB, whose other entries name 0x801EF784,
 *   0x801EF7D4, 0x801EF8A0, 0x801EF8EC and 0x801EF974. That table carries no
 *   prototype, so the argument roles stay structural, and the roles of
 *   work-object bytes 0xA and 0x2 are not established, so both stay
 *   offset-anchored. Bytes 0x801EF850..0x801EF89F are not byte-identical to
 *   any other span in the 795 payload files under out/binaries (one scan; the
 *   only hit is this span at its own offset 0xC50), so the body is written
 *   from its own bytes, taking statement order, operand forms and cell
 *   spelling from the exact sibling
 *   incrementWorkByte9AndAdvanceWorkByte2At10BmagicMagic08603_801EF784, whose
 *   80 bytes differ only in the byte-0xA offset and the 0x16 comparison
 *   constant this span carries.
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementWorkByteAAndAdvanceWorkByte2At16BmagicMagic08603_801EF850(void) {
  if ((g_battle_work[0xA] += 1) == 0x16) {
    g_battle_work[2]++;
  }
}
