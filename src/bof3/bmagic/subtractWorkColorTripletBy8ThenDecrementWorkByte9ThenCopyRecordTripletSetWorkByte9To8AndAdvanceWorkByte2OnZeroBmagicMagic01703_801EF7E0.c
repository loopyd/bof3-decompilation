#include "bof3/bof3.h"

/* Shared battle work pointer in main RAM, outside this overlay's image. Splat
 * binds the address through this target's generated undefined_syms_auto.txt,
 * where the remaining asm spans still reference it, so this lift needs no
 * target-map row. The exact siblings of this overlay read the same record:
 * addWorkColorTripletBy8ThenDecrementWorkByte9ThenDecrementGlobalWorkByteBAnd
 * AdvanceWorkByte2OnZeroBmagicMagic01703_801EF8B4 reads its byte 0xB, and the
 * bmagic lift decrementWorkByte9ThenCopyWorkWordTripletClearWord14AndByte9SetA
 * To26AndAdvanceWorkByte2BmagicMagic22203_801EF634 copies the same words 0x34,
 * 0x38 and 0x3C out of it. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Scratchpad work-object pointer cell. Declared as a named fixed address, the
 * form the exact siblings use, so the assembler folds the constant address into
 * each access (lui $vN,0x1F80 / lw $vN,68($vN)); the original reaches it once
 * per colour channel and once for the countdown byte, then once more for the
 * copy target and once for the final handler advance, because a store through
 * the pointer it supplied may alias the cell. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind unknown */

/* @source 0x801EF7E0
 * @behavior Per-frame handler of this overlay's leading code pointer run: word
 *           10 of the run at 0x801EEC04, stored at 0x801EEC2C, is this
 *           function's address. Subtracts 8 from the scratchpad work object's
 *           three colour channels at bytes 0x5D, 0x5E and 0x5F, reading the
 *           work-object pointer cell at 0x1F800044 once per channel, then
 *           decrements that object's countdown byte 9 with a byte-width
 *           read-modify-write (lbu / addiu -1 / sb) and tests the truncated
 *           stored value against zero (andi 0xFF / bnez). Only when the
 *           countdown reaches zero it reads the shared battle record through
 *           0x801EB4E0, re-reads the work cell, copies the record's words 0x34,
 *           0x38 and 0x3C into the work object's words 0x34, 0x38 and 0x3C (the
 *           word 0x3C is held in a local so its load precedes the byte-9 store
 *           and its store follows it), reloads the work object's countdown byte
 *           9 to 8, and re-reads the cell once more to increment work byte 2 by
 *           one, which advances the dispatcher on work byte 2 to its next
 *           handler step. Leaf with no frame; takes no arguments and returns
 *           nothing, and reads or writes no other state. The colour and
 *           countdown groups are the mirror of the complete exact sibling
 *           addWorkColorTripletBy8ThenDecrementWorkByte9ThenDecrementGlobalWork
 *           ByteBAndAdvanceWorkByte2OnZeroBmagicMagic01703_801EF8B4, whose
 *           statement order and operand forms they follow with this function's
 *           own delta 8 and sign, and the zero-countdown copy group follows
 *           decrementWorkByte9ThenCopyWorkWordTripletClearWord14AndByte9SetATo
 *           26AndAdvanceWorkByte2BmagicMagic22203_801EF634 with this function's
 *           own byte-9 reload value.
 * @status exact
 * @match 100.00
 * @residual none
 */
void subtractWorkColorTripletBy8ThenDecrementWorkByte9ThenCopyRecordTripletSetWorkByte9To8AndAdvanceWorkByte2OnZeroBmagicMagic01703_801EF7E0(void) {
  u32 *source;
  u32 *target;
  u32 value;

  D_1F800044[0x5D] -= 8;
  D_1F800044[0x5E] -= 8;
  D_1F800044[0x5F] -= 8;

  if (--D_1F800044[9] == 0) {
    source = (u32 *)D_801EB4E0;
    target = (u32 *)D_1F800044;
    target[13] = source[13];
    target[14] = source[14];
    value = source[15];
    ((u8 *)target)[9] = 8;
    target[15] = value;
    D_1F800044[2]++;
  }
}
