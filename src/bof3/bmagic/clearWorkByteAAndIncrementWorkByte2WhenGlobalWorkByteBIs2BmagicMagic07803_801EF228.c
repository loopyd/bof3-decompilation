#include "bof3/bof3.h"

/* Shared battle work pointer in main RAM, outside this overlay's image. Splat
 * binds the address through this target's generated undefined_syms_auto.txt row
 * D_801EB4E0 = 0x801EB4E0, and the guard is byte-addressed, so the local
 * declaration keeps the byte view only. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Scratchpad work-object pointer cell shared by the battle and bmagic overlays.
 * This target's map binds it at 0x1F800044 as g_battle_work, the same binding
 * the exact sibling incrementWorkByte2WhenGlobalWorkByteBIs1BmagicMagic08603
 * _801F00C0 uses. The original re-materializes the cell address before each of
 * the two taken-path accesses (lui $vN,0x1F80 / lw $vN,0x44($vN)), so the two
 * statements stay on the named cell instead of sharing one loaded base. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EF228
 * @behavior Reads the shared battle work pointer at 0x801EB4E0 once and tests
 *           that record's byte at offset 0xB against two (lbu $v1 / addiu
 *           $v0,$zero,2 / bne, empty delay slot). Only when the byte equals 2
 *           it clears the scratchpad work-object byte at offset 0xA with sb
 *           $zero and then re-materializes the pointer cell at 0x1F800044 with
 *           its own lui/lw pair to increment that object's byte at offset 0x2
 *           at byte width (lbu / addiu $v0,$v0,1 / sb). Takes no arguments,
 *           returns nothing, keeps no frame and writes no other state; the
 *           scratchpad cell is dereferenced only on the taken path. The
 *           in-image code pointer run based at 0x801F0878 holds 0x801EF228 as
 *           its seventh word (index 6), so the entry is reached through that
 *           table and has no jal caller in the payload. The guard and the
 *           two-statement body are the exact shapes of
 *           clearWorkByte9AndIncrementWorkByte2WhenGlobalWorkByteBIs1Bmagic
 *           Magic06000_801F02CC (same byte width, same guard cell, same second
 *           cell reload, other guard constant and other cleared byte). No
 *           payload under out/binaries holds this 80-byte body elsewhere, so
 *           the clone lever is ineligible.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearWorkByteAAndIncrementWorkByte2WhenGlobalWorkByteBIs2BmagicMagic07803_801EF228(void) {
  if (D_801EB4E0[0xB] == 2) {
    g_battle_work[0xA] = 0;
    g_battle_work[2]++;
  }
}
