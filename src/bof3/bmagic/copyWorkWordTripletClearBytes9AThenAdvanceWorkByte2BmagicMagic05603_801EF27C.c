#include "bof3/bof3.h"

/* Battle record pointer published by the main executable. Splat binds the
 * address through this target's generated undefined_syms_auto.txt, where the
 * remaining asm spans func_801EEC94, func_801EF17C and func_801EF2DC still
 * reference it, so this lift needs no target-map row. The offsets read here
 * (words 0x34, 0x38 and 0x3C) are the ones the exact bmagic siblings
 * copyWorkWordTripletClearBytes9AThenAdvanceWorkByte2BmagicMagic10303_801F0344
 * and
 * decrementWorkByte9ThenCopyWorkWordTripletClearWord14AndByte9SetATo26AndAdvanceWorkByte2BmagicMagic22203_801EF634
 * copy from the record they read through their own record-global address; the
 * record layout is shared, the address is this overlay's own. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Scratchpad work-object pointer cell, bound at 0x1F800044 by this target's map
 * and shared by this overlay's dispatchers and handlers. Declared as a named
 * cell symbol, the form the exact sibling
 * copyRecordByte27ThenAdvanceWorkByte2BmagicMagic05603_801EFFE8 uses: the
 * symbolic address folds into each load (lui $vN,0x1F80 / lw $vN,68($vN)) and
 * the original re-reads the cell before every later store group, because a
 * store through the loaded pointer may alias the cell. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EF27C
 * @behavior Work-object step handler, reached through entry 4 of this overlay's
 *           handler table at 0x801F0744 (dispatched on work byte 1). Copies the
 *           words at 0x34, 0x38 and 0x3C of the battle record published at
 *           0x801EB4E0 into the scratchpad work object published at the pointer
 *           cell 0x1F800044, clears work byte 9 and work byte 0xA, and advances
 *           work byte 2 by one, which moves the work-byte-2 step on. The word at
 *           0x3C is held in a local so its load precedes the byte 9 clear and
 *           its store follows it, and the cell is re-read before that store and
 *           again before the byte 2 advance, as the original does. Leaf with no
 *           frame; takes no arguments, returns nothing and reads or writes no
 *           other state.
 * @status exact
 * @match 100.00
 * @residual none
 */
void copyWorkWordTripletClearBytes9AThenAdvanceWorkByte2BmagicMagic05603_801EF27C(void) {
  u8 *source;
  u8 *work;
  u8 *next;
  u8 *step;
  u32 value;

  source = D_801EB4E0;
  work = g_battle_work;
  *(u32 *)(work + 0x34) = *(u32 *)(source + 0x34);
  *(u32 *)(work + 0x38) = *(u32 *)(source + 0x38);
  value = *(u32 *)(source + 0x3C);
  work[9] = 0;
  next = g_battle_work;
  *(u32 *)(work + 0x3C) = value;
  next[0xA] = 0;
  step = g_battle_work;
  step[2]++;
}
