#include "bof3/bof3.h"

/* Shared battle work pointer in main RAM, outside this overlay's image. Splat
 * binds the address through this target's generated undefined_syms_auto.txt, so
 * no target-map row or WEAK_SYMBOL_AT binding is needed. The sibling bmagic
 * lifts read the same record through this pointer
 * (copyWorkWordTripletAndClearByte9ThenAdvanceWorkByte2BmagicMagic04103_801EF484
 * copies its words 0x34/0x38/0x3C into the same work words). */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Scratchpad work-object pointer cell, bound at 0x1F800044 by this target's
 * map. It is a named cell symbol, the form the exact siblings use: the symbolic
 * address lets the assembler fold it into each load (lui $vN,0x1F80 / lw
 * $vN,68($vN)), and every later evaluation is separated by a store through the
 * previously loaded pointer, so each one reloads the cell. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EF8C0
 * @behavior Overlay step handler of the work-byte-1 handler table at
 *           0x801F07D0 (entry 2): clears bit 0x40 of byte 0 of the scratchpad
 *           work object published at the pointer cell 0x1F800044, copies the
 *           words at 0x34, 0x38 and 0x3C of the shared battle work record read
 *           through 0x801EB4E0 into the same work offsets, sets work bytes 9
 *           and 0xA to 1, and advances work byte 2 with a byte-width
 *           read-modify-write (lbu/addiu/sb), which is the store in the return
 *           delay slot. The original re-reads the pointer cell before each
 *           later store group because the stores through the loaded pointer may
 *           alias the cell, so the flag clear keeps its own cell read and the
 *           0x34/0x38/0x3C word copies share the next one. Leaf with no frame;
 *           nothing is returned and no other state is read or written.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearWorkFlag40AndCopyRecordTripletSetBytes9ATo1ThenAdvanceWorkByte2BmagicMagic09203_801EF8C0(void) {
  s32 *src;
  s32 *dst;

  g_battle_work[0] &= 0xBF;
  src = (s32 *)D_801EB4E0;
  dst = (s32 *)g_battle_work;
  dst[13] = src[13];
  dst[14] = src[14];
  dst[15] = src[15];
  ((u8 *)dst)[9] = 1;
  g_battle_work[0xA] = 1;
  g_battle_work[2]++;
}
