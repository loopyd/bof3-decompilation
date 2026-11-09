#include "bof3/bof3.h"

extern int rand(void);

/* Shared battle record pointer in main RAM, outside this overlay's image. Splat
 * binds the address through the encoded symbol name, so no target-map row is
 * added here. The exact siblings decrementGlobalWorkByteBAndInvokeHelper
 * BmagicMagic01200_801EF670 and advanceWorkByteAStepWorkPositionPairByAngle
 * Word0CAddWord20ToWord14AccumulateIntoHalfword3ECountdownWorkByte9Advance
 * WorkByte2OnZeroAndDrawSpriteMode1BmagicMagic01200_801EF3C4 read the same
 * record, and the exact copyWorkWordPairAndHalfwordClearBytes9AThenAdvance
 * WorkByte2BmagicMagic10303_801EEF94 measures its word pair 0x34/0x38 and its
 * halfword 0x3E from the parallel record published at 0x8014638C. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Battle target byte in main RAM, outside this overlay's image. Its 0x40 bit
 * separates the enemy group from the party group; the exact sibling
 * seedWorkMotionFromBattlerRecordAndAdvanceWorkByte2BmagicMagic05403_801EF238
 * branches on the same bit to select the enemy or party action record array.
 * Bound through this target's generated undefined_syms_auto.txt. */
extern u8 D_80146384; /* @source 0x80146384 @kind unknown */

/* Party and enemy action record arrays in main RAM, outside this overlay's
 * image: the 0x140-byte-stride local records at 0x80145E90 and the
 * 0x118-byte-stride enemy records at 0x801EB630 that the battle targets own.
 * This lift reads only byte 0x01 of one row, so both stay byte arrays with the
 * stride written at the index, the spelling the exact siblings
 * publishWorkRecordSelectSequenceByte8Plus3CThenAdvanceWorkByte3BmagicMagic151
 * 03_801EEEA8 and selectWorkRecordClearFlag40ThenAdvanceWorkByte1AndClear
 * WorkBytes2And3BmagicMagic15103_801EF2A0 use for the same two arrays. Splat
 * binds both addresses through this target's generated undefined_syms_auto.txt,
 * which the remaining asm span func_801EF188 still references. */
extern u8 D_80145E90[]; /* @source 0x80145E90 @kind table */
extern u8 D_801EB630[]; /* @source 0x801EB630 @kind table */

/* Scratchpad work-object pointer cell. The original re-materialises the cell
 * with its own lui 0x1F80 plus lw 0x44 pair at every read, because a store
 * through the loaded pointer may alias the cell, so the cell is named rather
 * than written as a bare scratchpad constant. The exact same-target siblings
 * advanceWorkByteAStepWorkPositionPairByAngleWord0CAddWord20ToWord14Accumulate
 * IntoHalfword3ECountdownWorkByte9AdvanceWorkByte2OnZeroAndDrawSpriteMode1
 * BmagicMagic01200_801EF3C4 and advanceWorkByteAOnFrameParityInvokePointHelper
 * CountdownWorkByte9DecrementWorkByte2AndDrawSpriteBmagicMagic01200_801EF6A4
 * declare the same cell the same way. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Battler-status helper and work-object setup helper of the battle image, both
 * outside this overlay's image and bound through this target's generated
 * undefined_funcs_auto.txt, which the remaining asm spans func_801EEC5C,
 * func_801EF080 and func_801EF188 still reference. The status helper reports
 * whether one battler slot is unavailable, either because the slot is inactive
 * or because its 0x4000 flag is set (the exact battle lift of 0x801DB524). The
 * helper at 0x801DD350 is reached here with the work object as its only
 * argument. */
extern u8 func_801DB524(u8 arg0); /* @source 0x801DB524 */
extern void func_801DD350(u8 *work); /* @source 0x801DD350 */

/* @source 0x801EF4B8
 * @behavior Work-byte-2 handler, the body the payload words at 0x801EFBC0,
 *           0x801EFBCC and 0x801EFBD8 store in the twelve-entry handler run at
 *           0x801EFBB4 that the dispatcher at 0x801EF03C indexes with the
 *           scratchpad work object's byte 0x2, so that byte holds 3, 6 or 9
 *           while this body runs. It first passes the work object's byte 0x3,
 *           the battler slot, to the shared status helper at 0x801DB524: when
 *           that slot is reported unavailable it stores 0xA into work byte 0x2,
 *           the selector of the sequential handler at 0x801EF670, and returns.
 *           Otherwise it reads byte 0x01 of the action record selected by work
 *           byte 0x4, from the 0x118-byte-stride enemy records at 0x801EB630
 *           when bit 0x40 of the battle target byte 0x80146384 is set and from
 *           the 0x140-byte-stride party records at 0x80145E90 otherwise, and
 *           returns without storing anything when that byte is 6. It then seeds
 *           the work object from the shared battle record at 0x801EB4E0: record
 *           byte 0x08 is copied into work byte 0x08, the motion pair at 0xC and
 *           0x10 takes 0x10000 and 0, the shared helper at 0x801DD350 is invoked
 *           on the work object, work words 0x34 and 0x38 take the record's same
 *           words added onto that new motion pair, work halfword 0x3E takes the
 *           record's 0x3E plus 0x100, work byte 0xB takes the low three bits of
 *           rand(), work byte 0x9 takes 0x10, work byte 0xA is cleared and work
 *           byte 0x2 advances by one. Each work-object access re-reads the
 *           pointer cell after a store through the previously loaded pointer,
 *           as the original does. The record byte compared against 6 is held in
 *           a word-width local, because the original compares the loaded byte
 *           without truncating it to byte width, and the setup pointer is a
 *           local of its own so that it reaches the helper in the argument
 *           register without a copy. Takes no arguments and returns nothing; the
 *           0x18-byte frame keeps $ra and the guard flag, whose zero
 *           initialization is scheduled into the first call's delay slot.
 * @status exact
 * @match 100.00
 * @residual none
 */
void seedWorkObjectFromBattlerRecordWhenBattlerReadyAndStateByteNotSixElseFinishWorkByte2BmagicMagic01200_801EF4B8(void) {
  s32 state;
  u8 proceed;
  u8 *work;
  u8 *source;
  u8 *motion;

  proceed = 0;
  if (func_801DB524(g_battle_work[3])) {
    g_battle_work[2] = 0xA;
    return;
  }
  if (D_80146384 & 0x40) {
    state = D_801EB630[g_battle_work[4] * 0x118 + 1];
  } else {
    state = D_80145E90[g_battle_work[4] * 0x140 + 1];
  }
  if (state != 6) {
    proceed = 1;
  }
  if (!proceed) {
    return;
  }

  g_battle_work[8] = D_801EB4E0[8];
  motion = g_battle_work;
  *(u32 *)(motion + 0xC) = 0x10000u;
  *(u32 *)(motion + 0x10) = 0;
  func_801DD350(motion);

  source = D_801EB4E0;
  work = g_battle_work;
  *(u32 *)(work + 0x34) = *(u32 *)(source + 0x34) + *(u32 *)(work + 0xC);
  *(u32 *)(work + 0x38) = *(u32 *)(source + 0x38) + *(u32 *)(work + 0x10);
  *(u16 *)(work + 0x3E) = *(u16 *)(source + 0x3E) + 0x100;

  g_battle_work[0xB] = rand() & 7;
  g_battle_work[9] = 0x10;
  g_battle_work[0xA] = 0;
  g_battle_work[2]++;
}
