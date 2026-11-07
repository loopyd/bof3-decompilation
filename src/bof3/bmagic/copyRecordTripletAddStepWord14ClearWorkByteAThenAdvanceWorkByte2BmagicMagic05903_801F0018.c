#include "bof3/bof3.h"

/* Battle record pointer published by the main executable. Splat binds the
 * address through this target's generated undefined_syms_auto.txt, where the
 * remaining asm spans of this overlay still reference it, so this lift needs no
 * target-map row. The words read here (0x34, 0x38 and 0x3C) are the position
 * triplet the exact bmagic siblings
 * copyWorkWordTripletClearBytes9AThenAdvanceWorkByte2BmagicMagic10303_801F0344
 * and decrementWorkByteASubtractWorkWord3CThenAdvanceWorkByte2OnZeroBmagicMagic12503_801EF0F8
 * read from that record and from the work object; the record layout is shared,
 * the address is this target's own. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Scratchpad work-object pointer cell, bound at 0x1F800044 by this target's map.
 * Declared as a named symbol so each read is a lui plus offset load: the
 * original re-reads the cell after the byte 0xA store, because a store through
 * the loaded work pointer may alias the cell (same reasoning as the exact
 * sibling copyWorkWordTripletClearBytes9AThenAdvanceWorkByte2BmagicMagic10303_801F0344). */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801F0018
 * @behavior Per-frame handler step of this overlay. It copies the position
 * triplet at words 0x34, 0x38 and 0x3C of the battle record published at
 * 0x801EB4E0 into the scratchpad work object published by the pointer cell
 * 0x1F800044, then advances the copied 0x3C word by the work object's own
 * 0x14 word, the 16.16 motion step the lifted siblings add to that position.
 * The record word 0x3C is held in a local, its 0x14 step is read into a second
 * local, the work object's 0xA byte is cleared with a byte store, and only then
 * is the summed 0x3C word stored, so both loads precede the byte clear while
 * the store follows the cell re-read as the original schedules them. The cell
 * is re-read once for the closing step: it reads work byte 2 with lbu, adds one
 * and stores it back in the return delay slot, advancing the dispatcher to the
 * next step. Leaf with no frame; takes no arguments, returns nothing and reads
 * or writes no other state.
 * @status exact
 * @match 100.00
 * @residual none
 */
void copyRecordTripletAddStepWord14ClearWorkByteAThenAdvanceWorkByte2BmagicMagic05903_801F0018(void) {
  u8 *source;
  u8 *work;
  u8 *next;
  u32 value;
  u32 step;

  source = D_801EB4E0;
  work = g_battle_work;
  *(u32 *)(work + 0x34) = *(u32 *)(source + 0x34);
  *(u32 *)(work + 0x38) = *(u32 *)(source + 0x38);
  value = *(u32 *)(source + 0x3C);
  step = *(u32 *)(work + 0x14);
  work[0xA] = 0;
  next = g_battle_work;
  *(u32 *)(work + 0x3C) = value + step;
  next[2]++;
}
