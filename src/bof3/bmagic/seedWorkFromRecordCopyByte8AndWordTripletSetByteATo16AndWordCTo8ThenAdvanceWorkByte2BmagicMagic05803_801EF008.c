#include "bof3/bof3.h"

/* Battle record pointer published by the main executable at 0x801EB4E0, outside
 * this overlay image and already bound by this target's generated
 * undefined_syms_auto.txt as D_801EB4E0, so no target-map row is needed. The
 * word it holds is read twice in this body - once for the byte 0x08 copy and
 * once for the word triplet at 0x34/0x38/0x3C - and the original re-reads the
 * cell for the second group because the byte store made through the work
 * pointer may alias it. The record layout is the one the bmagic sibling
 * copyWorkWordTripletSetBytes9To4AndAToB0ThenAdvanceWorkByte2BmagicMagic07903_801EFC74
 * copies through its own record pointer, and the second group takes the same
 * per-group locals there. The head is written as one direct expression pair so
 * its two address loads land in the first two temporaries, which is where this
 * span keeps them; spelling the head with its own source and work locals moved
 * both loads one register up and measured 15/30 instructions. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Scratchpad work-object pointer cell at 0x1F800044, bound by this target's own
 * map and read by its other exact lifts clearWorkBytesBmagicMagic05803_801EFE9C
 * and recordWithinWorkPositionRadiusBmagicMagic05803_801EFDD4. Declared as a
 * named cell symbol so every read is a lui plus offset load, and the original
 * re-reads the cell after the byte 0x0A store, which is where the source takes
 * its second local. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EF008
 * @behavior Overlay step body between func_801EEF74 and func_801EF080 that no
 *           word and no jal in this payload holds, so no in-image table entry or
 *           call reaches it and its selector lives outside the payload. It seeds
 *           the scratchpad work object published at the pointer cell 0x1F800044
 *           from the battle record published at 0x801EB4E0: work byte 0x08 takes
 *           the record byte 0x08, work words 0x34, 0x38 and 0x3C take the record
 *           words at the same offsets, work byte 0x0A takes 0x10 and the work
 *           word at 0x0C takes 8. Both cells are read again for the word triplet
 *           group, the record word at 0x3C is held in a local so its load
 *           precedes the byte 0x0A store, and the work cell is read once more
 *           after that store, before the two word stores. It finishes by
 *           advancing work byte 0x02 by one. Leaf with no frame, no arguments and
 *           no return value; it reads or writes no other state.
 * @status exact
 * @match 100.00
 * @residual none
 */
void seedWorkFromRecordCopyByte8AndWordTripletSetByteATo16AndWordCTo8ThenAdvanceWorkByte2BmagicMagic05803_801EF008(void) {
  u8 *source;
  u8 *work;
  u8 *next;
  u32 value;

  g_battle_work[0x08] = D_801EB4E0[0x08];
  source = D_801EB4E0;
  work = g_battle_work;
  *(u32 *)(work + 0x34) = *(u32 *)(source + 0x34);
  *(u32 *)(work + 0x38) = *(u32 *)(source + 0x38);
  value = *(u32 *)(source + 0x3C);
  work[0x0A] = 0x10;
  next = g_battle_work;
  *(u32 *)(work + 0x0C) = 8;
  *(u32 *)(work + 0x3C) = value;
  next[0x02]++;
}
