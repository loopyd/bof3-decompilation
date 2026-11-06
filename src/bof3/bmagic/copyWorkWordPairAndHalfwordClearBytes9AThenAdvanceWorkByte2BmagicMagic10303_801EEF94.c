#include "bof3/bof3.h"

/* Battle record pointer published by the main executable. Splat binds the
 * address through this target's generated undefined_syms_auto.txt, where the
 * remaining asm spans func_801EFB88 and func_801F0344 still reference it, so
 * this lift needs no target-map row. The offsets read here (words 0x34 and
 * 0x38, halfword 0x3E) are the ones the exact bmagic sibling
 * seedWorkObjectFromBattleRecordAndAdvanceWorkByte2BmagicMagic04500_801EF4A4
 * copies from the record it reads through 0x801EB4E0; the record layout is
 * shared, the address is this target's own. */
extern u8 *D_8014638C; /* @source 0x8014638C @kind unknown */

/* Scratchpad work-object pointer cell, bound by this target's map. Declared as
 * a named symbol so every access is a lui plus offset load: the original
 * re-reads the cell before each later store group, because a store through the
 * loaded pointer may alias the cell (same reasoning as the exact siblings
 * seedWorkObjectFromBattleRecordAndAdvanceWorkByte2BmagicMagic04500_801EF4A4
 * and copyWorkTripletSeedOffsetPairAndAdvanceWorkByte2BmagicMagic10503_801EF06C). */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EEF94
 * @behavior Copies the words at 0x34 and 0x38 and the halfword at 0x3E of the
 *           battle record published at 0x8014638C into the scratchpad work
 *           object published at the pointer cell 0x1F800044, clears work byte
 *           9 and work byte 0xA, and advances work byte 2 by one. The halfword
 *           is held in a local so its store follows the byte 9 clear, and the
 *           cell is re-read before that store and before the byte 0xA clear, as
 *           the original does. Leaf with no frame; takes no arguments, returns
 *           nothing and reads or writes no other state.
 * @status exact
 * @match 100.00
 * @residual none
 */
void copyWorkWordPairAndHalfwordClearBytes9AThenAdvanceWorkByte2BmagicMagic10303_801EEF94(void) {
  u8 *source;
  u8 *work;
  u8 *next;
  u8 *step;
  u16 distance;

  source = D_8014638C;
  work = g_battle_work;
  *(u32 *)(work + 0x34) = *(u32 *)(source + 0x34);
  *(u32 *)(work + 0x38) = *(u32 *)(source + 0x38);
  distance = *(u16 *)(source + 0x3E);
  work[9] = 0;
  next = g_battle_work;
  *(u16 *)(work + 0x3E) = distance;
  next[0xA] = 0;
  step = g_battle_work;
  step[2]++;
}
