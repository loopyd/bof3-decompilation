#include "bof3/bof3.h"

/* Battle record pointer published by the main executable. Splat binds the
 * address through this target's generated undefined_syms_auto.txt, where the
 * remaining asm span func_801EEE64 still references it, so this lift needs no
 * target-map row. The offsets read here (words 0x34, 0x38 and 0x3C) are the
 * ones the exact bmagic siblings
 * copyWorkWordTripletClearBytes9AThenAdvanceWorkByte2BmagicMagic10303_801F0344
 * and copyWorkWordTripletAndClearByte9ThenAdvanceWorkByte2BmagicMagic04103_801EF484
 * copy from the record they read through 0x801EB4E0; the record layout is
 * shared, the address is this target's own. */
extern u8 *D_8014638C; /* @source 0x8014638C @kind unknown */

/* Scratchpad work-object pointer cell, bound at 0x1F800044 by this target's
 * map. Declared as a named cell symbol, the form the exact siblings use: the
 * symbolic address folds into each load (lui $vN,0x1F80 / lw $vN,68($vN)), and
 * the original re-reads the cell before each later store group because a store
 * through the loaded pointer may alias the cell. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EFC74
 * @behavior Overlay step handler, entry 46 of the work-byte-1 dispatch table
 *           based at 0x801F02A4: copies the words at 0x34, 0x38 and 0x3C of the
 *           battle record published at 0x8014638C into the scratchpad work
 *           object published at the pointer cell 0x1F800044, sets countdown
 *           byte 9 to 4 and work byte 0xA to 0xB0, then advances work byte 2 by
 *           one. The word at 0x3C is held in a local so its load precedes the
 *           byte 9 store and its store follows it, and the cell is re-read
 *           after that store and again before the byte 2 advance, as the
 *           original does. Leaf with no frame; takes no arguments, returns
 *           nothing and reads or writes no other state.
 * @status exact
 * @match 100.00
 * @residual none
 */
void copyWorkWordTripletSetBytes9To4AndAToB0ThenAdvanceWorkByte2BmagicMagic07903_801EFC74(void) {
  u8 *source;
  u8 *work;
  u8 *next;
  u8 *step;
  u32 value;

  source = D_8014638C;
  work = g_battle_work;
  *(u32 *)(work + 0x34) = *(u32 *)(source + 0x34);
  *(u32 *)(work + 0x38) = *(u32 *)(source + 0x38);
  value = *(u32 *)(source + 0x3C);
  work[9] = 4;
  *(u32 *)(work + 0x3C) = value;
  next = g_battle_work;
  next[0xA] = 0xB0;
  step = g_battle_work;
  step[2]++;
}
