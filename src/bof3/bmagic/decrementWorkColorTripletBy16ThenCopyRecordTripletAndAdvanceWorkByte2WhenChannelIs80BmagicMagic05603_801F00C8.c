#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell shared by this overlay's dispatchers and
 * handlers. Typed as the same volatile u8 pointer the exact bmagic siblings use
 * (copyRecordByte27ThenAdvanceWorkByte2BmagicMagic05603_801EFFE8 reads the same
 * cell): every statement that goes through it must re-read the cell, because a
 * store through the pointer it supplied may alias the cell itself, and the
 * original reloads it before each of the three channel stores and again before
 * the tint test. The named symbol lets the assembler fold the constant address
 * into each load (lui $vN,0x1F80 / lw $vN,68($vN)). */
extern u8* volatile g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared battle work pointer in main RAM, outside this overlay's image. The
 * bmagic siblings read the same record through this pointer, and this entry
 * copies its words 0x34, 0x38 and 0x3C. Splat binds the address through this
 * target's generated undefined_syms_auto.txt, so no target-map row is needed. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* @source 0x801F00C8
 * @behavior Unconditional fade step of the overlay work object: it reads the
 *           scratchpad work-object pointer cell at 0x1F800044 three times and
 *           subtracts 0x10 from each of the work object's three colour channels
 *           at 0x5D, 0x5E and 0x5F at byte width, with no floor test before the
 *           stores. It then re-reads the cell and the signed channel at 0x5D,
 *           and when that channel now reads exactly -0x80 it copies words 0x34,
 *           0x38 and 0x3C of the shared battle work record read through
 *           0x801EB4E0 into the work object and increments that object's handler
 *           byte 2 with a byte-width add (lbu / addiu 1 / sb), advancing to the
 *           next handler. The record is read once and the work object is
 *           addressed through the pointer the channel test loaded, with the word
 *           0x3C copy last. Leaf with no arguments and no return value and no
 *           stack frame; reads and writes no other state.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementWorkColorTripletBy16ThenCopyRecordTripletAndAdvanceWorkByte2WhenChannelIs80BmagicMagic05603_801F00C8(void) {
  u8 *work;
  s32 *source;
  s32 *target;

  g_battle_work[0x5D] -= 0x10;
  g_battle_work[0x5E] -= 0x10;
  g_battle_work[0x5F] -= 0x10;

  work = g_battle_work;
  if (((s8 *)work)[0x5D] == -0x80) {
    source = (s32 *)D_801EB4E0;
    target = (s32 *)work;
    target[13] = source[13];
    target[14] = source[14];
    target[15] = source[15];
    ((u8 *)target)[2]++;
  }
}
