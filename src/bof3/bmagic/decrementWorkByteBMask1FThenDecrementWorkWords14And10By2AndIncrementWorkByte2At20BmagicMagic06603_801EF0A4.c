#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell at 0x1F800044. This target's map binds
 * that address to this name, so each access materialises the base itself
 * (lui $vN,0x1F80 / lw $vN,0x44($vN)) the way the original does, instead of
 * sharing one constant-address base across both reads. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EF0A4
 * @behavior Entry 5 of this overlay's ten-address handler table at
 *           0x801F0064, which the dispatcher at 0x801EEEBC indexes with the
 *           work object's byte 0x1, so byte 0x1 holds 5 while this body runs.
 *           Decrements the scratchpad work object's byte 0xB and keeps only its
 *           low five bits (lbu / addiu -1 / andi 0x1F / sb), so that counter
 *           wraps from zero to 0x1F. It then re-reads the pointer cell at
 *           0x1F800044 and decrements two of the work object's words by two:
 *           word 0x14 (lw / addiu -2 / sw) and word 0x10 (lw / addiu -2 / sw),
 *           both stored unconditionally, the word 0x10 store landing in the
 *           branch delay slot. When the decremented word 0x10 equals 0x20 the
 *           work object's byte 0x2 is incremented with a byte-width add (lbu /
 *           addiu 1 / sb), which advances the handler. Takes no arguments,
 *           returns nothing and keeps no frame. The guarded tail is the same
 *           shape as the exact sibling
 *           decrementWorkByteBMask3FThenAdvanceWorkWord1CBy2AndIncrementWorkByte2At80BmagicMagic13203_801EF388,
 *           which masks with 0x3F, advances word 0x1C by two and tests 0x80.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementWorkByteBMask1FThenDecrementWorkWords14And10By2AndIncrementWorkByte2At20BmagicMagic06603_801EF0A4(void) {
  u8 *work;
  u8 *cursor;
  s32 word14;
  s32 word10;

  work = g_battle_work;
  work[0xB] = (work[0xB] - 1) & 0x1F;
  cursor = g_battle_work;
  word14 = *(s32 *)(cursor + 0x14) - 2;
  word10 = *(s32 *)(cursor + 0x10) - 2;
  *(s32 *)(cursor + 0x14) = word14;
  *(s32 *)(cursor + 0x10) = word10;
  if (word10 == 0x20) {
    cursor[2]++;
  }
}
