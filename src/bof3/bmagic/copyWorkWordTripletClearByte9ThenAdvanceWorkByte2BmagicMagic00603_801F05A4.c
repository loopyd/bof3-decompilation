#include "bof3/bof3.h"

/* Battle record pointer published by the main executable. Splat binds the
 * address through this target's generated undefined_syms_auto.txt, where the
 * remaining asm spans func_801EF118 and func_801F0100 still reference it; the
 * lift at 0x801EEF28 was a third one of those spans before its own lift, so
 * this lift needs no target-map row. The word offsets read here
 * (0x34, 0x38 and 0x3C) are the ones the exact bmagic siblings
 * copyWorkWordTripletClearBytes9AThenAdvanceWorkByte2BmagicMagic10303_801F0344
 * and copyWorkWordPairAndHalfwordClearBytes9AThenAdvanceWorkByte2BmagicMagic10303_801EEF94
 * copy out of the record they read through 0x8014638C: the record layout is
 * shared and the address is not this target's own. */
extern u8 *D_8014638C; /* @source 0x8014638C @kind unknown */

/* Scratchpad work-object pointer cell, bound at 0x1F800044 by this target's map.
 * It is a named cell symbol, the form the exact siblings use: the symbolic
 * address lets the assembler fold it into each evaluation (lui 0x1F80 plus
 * lw 68($vN)), and the original re-reads the cell after a store through the
 * pointer it published, because that store may alias the cell. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801F05A4
 * @behavior Copies the words at 0x34, 0x38 and 0x3C of the battle record
 *           published at 0x8014638C into the scratchpad work object published at
 *           the pointer cell 0x1F800044, clears work byte 9 and advances work
 *           byte 2 by one. The word at 0x3C is held in a local so its load
 *           precedes the byte 9 clear and its store follows it, and the cell is
 *           re-read between that clear and that store, as the original does, so
 *           the byte 2 read-modify-write uses the re-read base instead of the
 *           cell. Leaf with no frame; takes no arguments, returns nothing and
 *           reads or writes no other state.
 * @status exact
 * @match 100.00
 * @residual none
 */
void copyWorkWordTripletClearByte9ThenAdvanceWorkByte2BmagicMagic00603_801F05A4(void) {
  u8 *source;
  u8 *work;
  u8 *next;
  u32 value;

  source = D_8014638C;
  work = g_battle_work;
  *(u32 *)(work + 0x34) = *(u32 *)(source + 0x34);
  *(u32 *)(work + 0x38) = *(u32 *)(source + 0x38);
  value = *(u32 *)(source + 0x3C);
  work[9] = 0;
  next = g_battle_work;
  *(u32 *)(work + 0x3C) = value;
  next[2]++;
}
