#include "bof3/bof3.h"

/* Shared battle work pointer in main RAM, outside this overlay's image. Splat
 * binds the address through this target's generated undefined_syms_auto.txt row
 * D_801EB4E0 = 0x801EB4E0, which the remaining asm spans func_801EEC90,
 * func_801EEF28, func_801EF298, func_801EF41C, func_801F0290 and func_801F0684
 * of this target still reference, so no target-map row is needed, and every
 * access here is byte-addressed, so the local declaration keeps the byte view
 * only. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Scratchpad work-object pointer cell shared by the battle and bmagic overlays;
 * this target's map binds it at 0x1F800044 as g_battle_work, the evidenced name
 * the exact lift copyWorkWordTripletClearByte9ThenAdvanceWorkByte2Bmagic
 * Magic00603_801F05A4 of this same target uses. The original re-materializes
 * the cell for the one access on the taken path (lui $v1,0x1F80 / lw
 * $v1,0x44($v1)), a fresh load after the guard pointer is already dead. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801F0644
 * @behavior Reads the shared battle work pointer at 0x801EB4E0 once and tests
 *           that record's byte at offset 0xB against one (lbu $v1 / addiu
 *           $v0,$zero,1 / bne, empty delay slot). Only when the byte equals 1
 *           it loads the scratchpad work-object pointer cell at 0x1F800044
 *           with its own lui/lw pair and increments that object's byte at
 *           offset 0x2 at byte width (lbu / addiu $v0,$v0,1 / sb). Takes no
 *           arguments, returns nothing, keeps no frame and writes no other
 *           state; the scratchpad cell is dereferenced only on the taken path.
 *           The pointer word at 0x801EEC30 holds this entry, index 11 of the
 *           13-word run 0x801EEC04..0x801EEC34 in this target's tabledata
 *           segment, so it is reached through that table and has no jal caller
 *           in the payload. Bytes 0x801F0644..0x801F0683 compare equal to the
 *           exact lifts incrementWorkByte2WhenGlobalWorkByteBIs1Bmagic
 *           Magic04103_801EF524 and incrementWorkByte2WhenGlobalWorkByteBIs1
 *           BmagicMagic08603_801F00C0 at their own payload offsets, so the
 *           statement forms and register choice are taken from those twins
 *           unchanged.
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementWorkByte2WhenGlobalWorkByteBIs1BmagicMagic00603_801F0644(void) {
  if (D_801EB4E0[0xB] == 1) {
    g_battle_work[2]++;
  }
}
