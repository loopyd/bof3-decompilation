#include "bof3/bof3.h"

/* Shared battle work pointer in main RAM, outside this overlay's image. Splat
 * binds the address through this target's generated undefined_syms_auto.txt, so
 * no target-map row or WEAK_SYMBOL_AT binding is needed. The sibling bmagic
 * lifts read the same record through this pointer
 * (seedWorkWordsAndCopyRecordByte27ClearByte9SetByteATo16ThenAdvanceWorkByte2
 * BmagicMagic00100_801EFBEC copies its byte 0x27). Its words 0x34, 0x38 and
 * 0x3C are the three words this handler copies out. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Scratchpad work-object pointer cell, the same named cell the exact sibling
 * seedWorkWordsAndCopyRecordByte27ClearByte9SetByteATo16ThenAdvanceWorkByte2
 * BmagicMagic00100_801EFBEC reads. A named cell symbol lets the assembler fold
 * the constant address into each load (lui $vN,0x1F80 / lw $vN,68($vN)), and the
 * original re-reads the cell after a store through the pointer it just loaded,
 * which a store through that loaded pointer forces. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind unknown */

/* @source 0x801EF5CC
 * @behavior Fade step of the overlay work object, registered as word 12
 *           (0x801EEC34) of this overlay's leading 24-word code pointer run at
 *           0x801EEC04. It reads the scratchpad work-object pointer cell at
 *           0x1F800044 and, while the object's signed tint channel at 0x5D has
 *           not reached its -0x80 floor, subtracts 0x10 from each of the three
 *           colour channels at 0x5D, 0x5E and 0x5F. The test and the 0x5D store
 *           share the first cell read; the 0x5E and 0x5F channels each re-read
 *           the cell, because a store through the pointer it supplied may alias
 *           it. It then re-reads the cell and reads the signed channel again -
 *           on the arm-skipped path that value is the one already loaded - and
 *           when the channel reads exactly -0x80 it copies words 0x34, 0x38 and
 *           0x3C of the shared battle work record read through 0x801EB4E0 into
 *           the work object and increments that object's handler byte 2 with a
 *           byte-width add (lbu / addiu 1 / sb), advancing to the next handler.
 *           The record is read once and the work object is addressed through the
 *           pointer the second channel read loaded, with the word 0x3C copy
 *           last. Leaf with no arguments and no return value; reads and writes
 *           no other state. The original reserves an 8-byte frame that no
 *           instruction outside the prologue and epilogue touches, and this body
 *           shape reserves those 8 bytes on its own: an extra 8-byte local on
 *           top of it measured a 0x10 frame with the two stack adjustments as
 *           the only difference.
 * @status exact
 * @match 100.00
 * @residual none
 */
void subtractWorkColorTripletBy16ThenCopyRecordTripletAndAdvanceWorkByte2WhenChannelIs80BmagicMagic00100_801EF5CC(void) {
  u8 *work;
  s32 *source;
  s32 *target;

  work = D_1F800044;
  if (((s8 *)work)[0x5D] != -0x80) {
    ((s8 *)work)[0x5D] -= 0x10;
    D_1F800044[0x5E] -= 0x10;
    D_1F800044[0x5F] -= 0x10;
  }

  if (((s8 *)D_1F800044)[0x5D] == -0x80) {
    source = (s32 *)D_801EB4E0;
    target = (s32 *)D_1F800044;
    target[13] = source[13];
    target[14] = source[14];
    target[15] = source[15];
    ((u8 *)target)[2]++;
  }
}
