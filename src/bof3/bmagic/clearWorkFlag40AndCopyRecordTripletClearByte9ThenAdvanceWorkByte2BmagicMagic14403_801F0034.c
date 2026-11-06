#include "bof3/bof3.h"

/* Shared battle work pointer in main RAM, outside this overlay's image. Splat
 * binds the address through this target's generated undefined_syms_auto.txt, so
 * no target-map row or WEAK_SYMBOL_AT binding is needed. The sibling bmagic
 * lifts read the same record through this pointer
 * (copyWorkWordTripletAndClearByte9ThenAdvanceWorkByte2BmagicMagic04103_801EF484
 * copies its words 0x34/0x38/0x3C into the same work offsets). */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Scratchpad work-object pointer cell, shared by this overlay's dispatchers and
 * handlers and bound at 0x1F800044 by this target's map. It is a named cell
 * symbol, the form the exact siblings use: the symbolic address lets the
 * assembler fold it into each load (lui $vN,0x1F80 / lw $vN,68($vN)), and every
 * later evaluation follows a store through the previously loaded pointer, so
 * each evaluation reloads the cell. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801F0034
 * @behavior Overlay step handler of the work-byte-1 table based at 0x801F04D0
 *           (entry 11): clears bit 0x40 of byte 0 of the scratchpad work object
 *           published at the pointer cell 0x1F800044, copies the words at
 *           0x34, 0x38 and 0x3C of the shared battle work record read through
 *           0x801EB4E0 into the same work offsets, clears work byte 9 (the
 *           original stores $zero with sb) and then re-reads the pointer cell
 *           and advances work byte 2 by one with a byte-width read-modify-write
 *           (lbu/addiu/sb), which is the store in the return delay slot. The
 *           cell is re-read before the word triplet and again before the byte 2
 *           bump because the stores through the loaded pointer may alias it.
 *           Leaf with no frame; nothing is returned and no other state is read
 *           or written.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearWorkFlag40AndCopyRecordTripletClearByte9ThenAdvanceWorkByte2BmagicMagic14403_801F0034(void) {
  u8 *work;
  s32 *src;
  s32 *dst;

  /* The flag group needs its own work pointer: a second assignment through the
   * copy group's variable merges the two quantities, which gives the leading
   * pointer group the copy group's register (a0) instead of the original's v1.
   */
  work = g_battle_work;
  work[0] &= 0xBF;
  src = (s32 *)D_801EB4E0;
  dst = (s32 *)g_battle_work;
  dst[13] = src[13];
  dst[14] = src[14];
  dst[15] = src[15];
  ((u8 *)dst)[9] = 0;
  g_battle_work[2]++;
}
