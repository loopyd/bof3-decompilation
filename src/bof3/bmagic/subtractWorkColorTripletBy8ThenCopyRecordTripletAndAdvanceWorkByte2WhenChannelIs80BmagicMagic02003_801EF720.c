#include "bof3/bof3.h"

/* Shared battle work pointer in main RAM, outside this overlay's image. Splat
 * binds the address through this target's generated undefined_syms_auto.txt, so
 * no target-map row is needed. The exact lifts of this target read the same
 * record (addWorkColorTripletBy16ThenSetGlobalWorkByteBToFFAndAdvanceWorkByte2
 * WhenChannelIsZeroBmagicMagic02003_801EF7D0 writes its byte 0xB), and its words
 * 0x34, 0x38 and 0x3C are the three words this handler copies out. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Scratchpad work-object pointer cell, spelled the same named cell the exact
 * lifts of this target already use. A named cell symbol lets the assembler fold
 * the constant address into each load (lui $vN,0x1F80 / lw $vN,0x44($vN)), and
 * the original re-reads the cell after each store through the pointer it
 * supplied, which a store through that loaded pointer forces. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind unknown */

/* @source 0x801EF720
 * @behavior Fade step of this overlay's handler run. It reads the scratchpad
 *           work-object pointer cell at 0x1F800044 three times and subtracts
 *           0x8 from each of the work object's colour channels at bytes 0x5D,
 *           0x5E and 0x5F at byte width, re-reading the cell before each
 *           channel because a store through the pointer it supplied may alias
 *           the cell. It then re-reads the cell and tests the signed channel at
 *           0x5D; when that channel reads exactly -0x80 (0x80 at byte width) it
 *           copies words 0x34, 0x38 and 0x3C of the shared battle work record
 *           read through 0x801EB4E0 into the work object and increments that
 *           object's handler byte 2 with a byte-width add (lbu / addiu 1 / sb),
 *           advancing the dispatcher on work byte 2 to its next handler step.
 *           The record is read once and the work object is addressed through
 *           the pointer the channel test loaded; the word 0x3C copy lands last
 *           and the byte 2 read is taken before that word's load, in the same
 *           register the original uses for it. Leaf with no arguments and no
 *           return value and no stack frame; reads and writes no other state.
 * @status exact
 * @match 100.00
 * @residual none
 */
void subtractWorkColorTripletBy8ThenCopyRecordTripletAndAdvanceWorkByte2WhenChannelIs80BmagicMagic02003_801EF720(void) {
  u8 *work;
  s32 *source;
  s32 *target;

  D_1F800044[0x5D] -= 0x8;
  D_1F800044[0x5E] -= 0x8;
  D_1F800044[0x5F] -= 0x8;

  work = D_1F800044;
  if (((s8 *)work)[0x5D] == -0x80) {
    source = (s32 *)D_801EB4E0;
    target = (s32 *)work;
    target[13] = source[13];
    target[14] = source[14];
    target[15] = source[15];
    ((u8 *)target)[2]++;
  }
}
