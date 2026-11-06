#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell at 0x1F800044. This target's map binds
 * that address to this name, so each access materialises the base itself
 * (lui $vN,0x1F80 / lw $vN,0x44($vN)) the way the original does, instead of
 * sharing one constant-address base across both reads. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EF388
 * @behavior Per-frame step handler of this overlay: entry 3 of the six-address
 *           handler table at 0x801EFD58, which the dispatcher at 0x801EF114
 *           indexes with byte 0x1 of the work object published at the
 *           scratchpad pointer cell 0x1F800044. Decrements the work object's
 *           byte 0xB and keeps only its low six bits (lbu / addiu -1 /
 *           andi 0x3F / sb), so that counter wraps from zero to 0x3F. It then
 *           re-reads the scratchpad pointer cell and advances the work
 *           object's word 0x1C by 2 (lw / addiu 2 / sw), writing the advanced
 *           word in the branch delay slot. When that new word equals 0x80 the
 *           sub-step byte 0x2 of the same object is incremented with a
 *           byte-width add (lbu / addiu 1 / sb). Takes no arguments, returns
 *           nothing and keeps no frame. Its guarded tail is instruction for
 *           instruction the body of the exact sibling
 *           advanceWorkWord14By80ThenIncrementWorkByte2At400BmagicMagic15103_801F1D28,
 *           which advances word 0x14 by 0x80 and tests 0x400 instead.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementWorkByteBMask3FThenAdvanceWorkWord1CBy2AndIncrementWorkByte2At80BmagicMagic13203_801EF388(void) {
  u8 *work;
  u8 *cursor;
  s32 value;

  work = g_battle_work;
  work[0xB] = (work[0xB] - 1) & 0x3F;
  cursor = g_battle_work;
  value = *(s32 *)(cursor + 0x1C) + 2;
  *(s32 *)(cursor + 0x1C) = value;
  if (value == 0x80) {
    cursor[2]++;
  }
}
