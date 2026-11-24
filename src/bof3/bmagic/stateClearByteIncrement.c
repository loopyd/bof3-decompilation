#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell shared by the battle and bmagic overlays.
 * This target's map binds it at 0x1F800044 as g_battle_work. The original
 * materializes the cell address once for the guard and the cleared byte, and
 * again after the store for the incremented byte, so the second statement stays
 * on the named cell instead of sharing the loaded base. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801F00A0
 * @behavior Reads the scratchpad work-object pointer cell at 0x1F800044 once,
 *           loads the record pointer its word 0x4C holds, and tests that
 *           record's byte 0xB against one (lw $v0,0x4C($a0) / lbu $v1,0xB($v0)
 *           / addiu $v0,$zero,1 / bne, empty delay slot). Only when the byte
 *           equals 1 it clears the work object's byte 0xA with sb $zero through
 *           the already loaded pointer and then re-materializes the cell with
 *           its own lui/lw pair to increment that object's byte at offset 0x2 at
 *           byte width (lbu / addiu $v0,$v0,1 / sb). Takes no arguments,
 *           returns nothing and keeps no frame. The record pointer at work word
 *           0x4C is written by func_801F0230, which stores 2 into that record's
 *           byte 0xB, so the guard reads the same state byte this overlay's
 *           shared-record guards read from 0x801EB4E0. No payload under
 *           out/binaries holds this 76-byte body elsewhere, so the clone lever
 *           is ineligible.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateClearByteIncrement(void) {
  u8 *work;

  work = g_battle_work;
  if (*(u8 *)(*(u32 *)(work + 0x4C) + 0xB) == 1) {
    work[0xA] = 0;
    g_battle_work[2]++;
  }
}
