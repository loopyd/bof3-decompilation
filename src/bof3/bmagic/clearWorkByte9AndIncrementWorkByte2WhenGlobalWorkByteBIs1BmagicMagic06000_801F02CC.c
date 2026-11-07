#include "bof3/bof3.h"

/* Shared battle work pointer in main RAM, outside this overlay's image. Splat
 * binds the address through this target's generated undefined_syms_auto.txt, and
 * the bytes read here are byte-addressed, so the local declaration keeps the
 * byte view only. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Scratchpad work-object pointer cell shared by the battle and bmagic overlays.
 * The original re-materializes the cell address before each of the two accesses
 * (lui $vN,0x1F80 / lw $vN,68($vN)), so the two statements stay on the named
 * cell instead of sharing one loaded base. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801F02CC
 * @behavior Reads the shared battle work pointer at 0x801EB4E0 and, only when
 * that work record's byte at offset 0xB equals 1, clears the scratchpad
 * work-object byte at offset 0x9 with sb $zero and then re-reads the
 * work-object pointer cell at 0x1F800044 and increments that object's byte at
 * offset 0x2 with a byte-width add (lbu, addiu $v0,$v0,1, sb). Nothing is
 * returned, no other state is read or written, the scratchpad cell is
 * dereferenced only on the taken path and the function has no frame. Composed
 * from two exact siblings in this same overlay: the guard is the
 * incrementWorkByte2WhenGlobalWorkByteBIs83BmagicMagic06000_801F0090 form with
 * this function's own immediate, and the two-statement body is
 * clearWorkByte9AndIncrementWorkByte2BmagicMagic06000_801F0000 unchanged.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearWorkByte9AndIncrementWorkByte2WhenGlobalWorkByteBIs1BmagicMagic06000_801F02CC(void) {
  if (D_801EB4E0[0xB] == 1) {
    g_battle_work[9] = 0;
    g_battle_work[2]++;
  }
}
