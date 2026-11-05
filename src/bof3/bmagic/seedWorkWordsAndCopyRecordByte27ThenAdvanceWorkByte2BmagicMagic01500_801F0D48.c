#include "bof3/bof3.h"

/* Shared battle work pointer in main RAM, outside this overlay's image. Splat
 * binds the address through the encoded symbol name, so no target-map row or
 * WEAK_SYMBOL_AT binding is needed. The sibling bmagic lifts read the same
 * record through this pointer (seedWorkObjectFromBattleRecordAndAdvanceWork
 * Byte2BmagicMagic04500_801EF4A4 copies its 0x34/0x38 words and its 0x3E
 * halfword, incrementWorkByte2WhenGlobalWorkByteBIs84BmagicMagic11303 reads
 * its byte 0xB). */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Scratchpad work-object pointer cell. Declared as a named symbol so every
 * access is a lui plus offset load, and the compiler re-reads the cell after a
 * store through the loaded pointer, which is what the original does before
 * each store group. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind unknown */

/* @source 0x801F0D48
 * @behavior Overlay handler seed: reads the scratchpad work object published
 *           at the pointer cell 0x1F800044 and the shared battle work record
 *           read through 0x801EB4E0, then writes the work object: word 0x14 =
 *           0x80, word 0x20 = -0x10, byte 0x27 = the record's byte 0x27, byte 9
 *           = 0x14, byte 0xA = 7, and finally advances work byte 2, the handler
 *           index, with a byte-width read-modify-write whose store the original
 *           places in the return delay slot. Each store group reads the pointer
 *           cell through its own local: the original reloads the cell before
 *           every group because a store through the loaded pointer may alias
 *           the cell, and one local reused for all groups collapses those loads
 *           onto a single register. The record pointer load precedes the word
 *           stores, while the narrow record byte load follows them.
 * @status exact
 * @match 100.00
 * @residual none
 */
void seedWorkWordsAndCopyRecordByte27ThenAdvanceWorkByte2BmagicMagic01500_801F0D48(void) {
  u8 *work;
  u8 *source;
  u8 *countdown;
  u8 *setA;
  u8 *step;

  work = D_1F800044;
  source = D_801EB4E0;
  *(u32 *)(work + 0x14) = 0x80;
  *(s32 *)(work + 0x20) = -0x10;
  work[0x27] = source[0x27];
  countdown = D_1F800044;
  countdown[9] = 0x14;
  setA = D_1F800044;
  setA[0xA] = 7;
  step = D_1F800044;
  step[2]++;
}
