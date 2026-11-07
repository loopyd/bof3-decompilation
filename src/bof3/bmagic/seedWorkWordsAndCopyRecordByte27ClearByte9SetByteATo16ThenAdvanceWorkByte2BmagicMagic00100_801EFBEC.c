#include "bof3/bof3.h"

/* Shared battle work pointer in main RAM, outside this overlay's image. Splat
 * binds the address through the encoded symbol name and this target's generated
 * undefined_syms_auto.txt, so no target-map row or WEAK_SYMBOL_AT binding is
 * needed. The sibling bmagic lifts read the same record through this pointer
 * (seedWorkWordsAndCopyRecordByte27ThenAdvanceWorkByte2BmagicMagic01500_801F0D48
 * copies its byte 0x27 the same way, seedWorkObjectFromBattleRecordAndAdvance
 * WorkByte2BmagicMagic04500_801EF4A4 copies its words 0x34/0x38). */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Scratchpad work-object pointer cell. Declared as a named symbol so every
 * access is a lui plus offset load, and the compiler re-reads the cell after a
 * store through the previously loaded pointer, which is what the original does
 * before each store group. The exact sibling
 * seedWorkWordsAndCopyRecordByte27ThenAdvanceWorkByte2BmagicMagic01500_801F0D48
 * reads the same cell in the same four groups through this spelling. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind unknown */

/* @source 0x801EFBEC
 * @behavior Overlay step handler seed: reads byte 0x27 of the shared battle
 *           work record published at 0x801EB4E0 and copies it into byte 0x27 of
 *           the scratchpad work object read through the pointer cell
 *           0x1F800044, then seeds that object: word 0x14 = 0x100, word 0x20 =
 *           -0x10, byte 9 = 0, byte 0xA = 0x10, and finally advances work byte
 *           2, the handler index, with a byte-width read-modify-write whose
 *           store the original places in the return delay slot (lbu/addiu/sb).
 *           The record pointer load precedes the first cell load, and the byte
 *           0x27 copy is its own store group ahead of the word stores. Each
 *           later store group reads the pointer cell through its own local: the
 *           original reloads the cell before every group because a store through
 *           the loaded pointer may alias the cell, and one local reused for all
 *           groups collapses those loads onto a single register. Leaf with no
 *           frame; nothing is returned and no other state is read or written.
 * @status exact
 * @match 100.00
 * @residual none
 */
void seedWorkWordsAndCopyRecordByte27ClearByte9SetByteATo16ThenAdvanceWorkByte2BmagicMagic00100_801EFBEC(void) {
  u8 *source;
  u8 *work;
  u8 *motion;
  u8 *setA;
  u8 *step;

  source = D_801EB4E0;
  work = D_1F800044;
  work[0x27] = source[0x27];
  motion = D_1F800044;
  *(u32 *)(motion + 0x14) = 0x100;
  *(s32 *)(motion + 0x20) = -0x10;
  motion[9] = 0;
  setA = D_1F800044;
  setA[0xA] = 0x10;
  step = D_1F800044;
  step[2]++;
}
