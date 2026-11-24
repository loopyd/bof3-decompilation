#include "bof3/bof3.h"

/* Shared battle work pointer in main RAM, outside this overlay's image. Splat
 * binds the address through this target's generated undefined_syms_auto.txt
 * row D_801EB4E0 = 0x801EB4E0, so no target-map row is needed, and every
 * access here is byte-addressed, so the local declaration keeps the byte view
 * only. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Scratchpad work-object pointer cell shared by the battle and bmagic overlays;
 * this target's map binds it at 0x1F800044 as g_battle_work, the same binding
 * the exact sibling incrementWorkByte2WhenGlobalWorkByteBIs1BmagicMagic08603
 * _801F00C0 uses. The original re-materializes the cell for the one access on
 * the taken path (lui $v1,0x1F80 / lw $v1,0x44($v1)), a fresh load after the
 * guard pointer is already dead. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EF3A0
 * @behavior Reads the shared battle work pointer at 0x801EB4E0 once and tests
 *           that record's byte at offset 0xB against two (lbu $v1 / addiu
 *           $v0,$zero,2 / bne, empty delay slot). Only when that byte equals 2
 *           it loads the scratchpad work-object pointer cell at 0x1F800044 with
 *           its own lui/lw pair and increments that object's byte at offset 0x2
 *           at byte width (lbu / addiu $v0,$v0,1 / sb). Takes no arguments,
 *           returns nothing, keeps no frame and writes no other state; the
 *           scratchpad cell is dereferenced only on the taken path. The pointer
 *           word at 0x801F0364, the second entry of workByte2HandlerTable at
 *           0x801F0360, is the only in-image reference to this address; no jal
 *           in the payload reaches it, which is the dispatch shape rather than
 *           a missing caller, so the argument roles stay structural. The
 *           0x40-byte body is instruction-for-instruction identical to the
 *           exact sibling incrementWorkByte2WhenGlobalWorkByteBIs1Bmagic
 *           Magic08603_801F00C0 except for the compared immediate at
 *           0x801EF3B0 (two here, one there), so the statement order, operand
 *           forms and register choice come from that sibling with this
 *           function's own constant. No byte-identical twin exists: a scan of
 *           every payload under out/binaries finds this 0x40-byte span once.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateIncrementOnTwo(void) {
  if (D_801EB4E0[0xB] == 2) {
    g_battle_work[2]++;
  }
}
