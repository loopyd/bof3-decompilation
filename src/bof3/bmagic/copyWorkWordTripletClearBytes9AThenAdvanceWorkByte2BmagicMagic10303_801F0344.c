#include "bof3/bof3.h"

/* Battle record pointer published by the main executable. Splat binds the
 * address through this target's generated undefined_syms_auto.txt, where the
 * remaining asm span func_801EFB88 still references it, so this lift needs no
 * target-map row. The offsets read here (words 0x34, 0x38 and 0x3C) are the ones
 * the exact bmagic sibling
 * seedWorkObjectFromBattleRecordAndAdvanceWorkByte2BmagicMagic04500_801EF4A4
 * copies from the record it reads through 0x801EB4E0; the record layout is
 * shared, the address is this target's own. */
extern u8 *D_8014638C; /* @source 0x8014638C @kind unknown */

/* Scratchpad work-object pointer cell, bound by this target's map. Declared as
 * a named symbol so every access is a lui plus offset load: the original
 * re-reads the cell before each later store group, because a store through the
 * loaded pointer may alias the cell (same reasoning as the exact siblings
 * seedWorkObjectFromBattleRecordAndAdvanceWorkByte2BmagicMagic04500_801EF4A4
 * and copyWorkWordPairAndHalfwordClearBytes9AThenAdvanceWorkByte2BmagicMagic10303_801EEF94). */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801F0344
 * @behavior Copies the words at 0x34, 0x38 and 0x3C of the battle record
 *           published at 0x8014638C into the scratchpad work object published
 *           at the pointer cell 0x1F800044, clears work byte 9 and work byte
 *           0xA, and advances work byte 2 by one. The word at 0x3C is held in a
 *           local so its load precedes the byte 9 clear and its store follows
 *           it, and the cell is re-read before that store and before the byte
 *           0xA clear, as the original does. Leaf with no frame; takes no
 *           arguments, returns nothing and reads or writes no other state.
 * @status exact
 * @match 100.00
 * @residual none
 */
void copyWorkWordTripletClearBytes9AThenAdvanceWorkByte2BmagicMagic10303_801F0344(void) {
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
  work[9] = 0;
  next = g_battle_work;
  *(u32 *)(work + 0x3C) = value;
  next[0xA] = 0;
  step = g_battle_work;
  step[2]++;
}
