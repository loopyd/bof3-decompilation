#include "bof3/bof3.h"

/* Shared battle work pointer in main RAM, outside this overlay's image. Splat
 * binds the address through this target's generated undefined_syms_auto.txt,
 * and every access here is byte-addressed, so the local declaration keeps the
 * byte view only. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Scratchpad work-object pointer cell shared by the battle and bmagic overlays;
 * this target's map binds it at 0x1F800044. The original re-materializes the
 * cell for the one access on the taken path (lui $v1,0x1F80 / lw $v1,0x44($v1)),
 * a fresh load after the guard pointer is already dead. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EF524
 * @behavior Reads the shared battle work pointer at 0x801EB4E0 once and tests
 *           that record's byte at offset 0xB against one (lbu $v1 / addiu
 *           $v0,$zero,1 / bne, empty delay slot). Only when the byte equals 1
 *           it loads the scratchpad work-object pointer cell at 0x1F800044
 *           with its own lui/lw pair and increments that object's byte at
 *           offset 0x2 at byte width (lbu / addiu $v0,$v0,1 / sb). Takes no
 *           arguments, returns nothing, keeps no frame and writes no other
 *           state; the scratchpad cell is dereferenced only on the taken path.
 *           Same shape and register choice as the exact sibling
 *           incrementWorkByte2WhenGlobalWorkByteBIs84BmagicMagic11303_801EFD00,
 *           with this function's own compared constant.
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementWorkByte2WhenGlobalWorkByteBIs1BmagicMagic04103_801EF524(void) {
  if (D_801EB4E0[0xB] == 1) {
    g_battle_work[2]++;
  }
}
