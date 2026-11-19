#include "bof3/bof3.h"

/* Shared battle work pointer in main RAM, outside this overlay's image. Splat
 * binds the address through this target's generated undefined_syms_auto.txt
 * row D_801EB4E0 = 0x801EB4E0, so no target-map row is needed, and every
 * access here is byte-addressed, so the local declaration keeps the byte view
 * only. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Scratchpad work-object pointer cell shared by the battle and bmagic overlays;
 * this target's map binds it at 0x1F800044 as g_battle_work, the same binding
 * the exact sibling seedRandomRadiusAndProjectWorkPositionPairThenAdvance
 * BmagicMagic09303_801EF4C8 uses. The original re-materializes the cell for the
 * one access on the taken path (lui $v1,0x1F80 / lw $v1,0x44($v1)), a fresh
 * load after the guard pointer is already dead. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EF710
 * @behavior Reads the shared battle work pointer at 0x801EB4E0 once and tests
 *           that record's byte at offset 0xB against eight (lbu $v1 / addiu
 *           $v0,$zero,8 / bne, empty delay slot). Only when the byte equals
 *           eight it loads the scratchpad work-object pointer cell at
 *           0x1F800044 with its own lui/lw pair and increments that object's
 *           byte at offset 0x2 at byte width (lbu / addiu $v0,$v0,1 / sb).
 *           Takes no arguments, returns nothing, keeps no frame and writes no
 *           other state; the scratchpad cell is dereferenced only on the taken
 *           path. The trailing pointer run at 0x801F089C holds 0x801EF710 at
 *           0x801F08BC, its ninth word, so the entry is one of the fourteen
 *           state handlers that run reaches and it has no jal caller in the
 *           payload. Bytes 0x801EF710..0x801EF74F were byte-identical to the
 *           then-exact lift
 *           incrementWorkByte2WhenGlobalWorkByteBIs1BmagicMagic08603_801F00C0
 *           except for the guard immediate's low byte (eight here, one there),
 *           so the statement order, operand forms and register choice were
 *           taken from that twin and the literal was written from these bytes.
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementWorkByte2WhenGlobalWorkByteBIs8BmagicMagic09303_801EF710(void) {
  if (D_801EB4E0[0xB] == 8) {
    g_battle_work[2]++;
  }
}
