#include "bof3/bof3.h"

/* Party action record row: the 0x140-byte rows the battle target indexes as
 * BattlePartyActionRecord (include/bof3/battle/battle15_internal.h), whose
 * first three words are the battler coordinate triplet. That header is the
 * battle target's private declaration set, so this overlay binds the row shape
 * it reads. The sibling seedWorkMotionFromBattlerRecordAndAdvanceWorkByte2BmagicMagic05403_801EF238,
 * measured exact when this seed was written, read the same two arrays with the
 * same coordinate-triplet interpretation of words 0, 4 and 8 and the same
 * strides, so the row names here follow it. */
typedef struct BmagicMagic05803PartyActionRow {
  u32 coord_00;
  u32 coord_04;
  u32 coord_08;
  u8 unk_0C[0x140 - 0x0C];
} BmagicMagic05803PartyActionRow;

/* Enemy action record row: the 0x118-byte rows the battle target indexes as
 * BattleEnemyActionRecord, whose first three words are that record's
 * coordinate triplet. */
typedef struct BmagicMagic05803EnemyActionRow {
  u32 coord_00;
  u32 coord_04;
  u32 coord_08;
  u8 unk_0C[0x118 - 0x0C];
} BmagicMagic05803EnemyActionRow;

/* Party action record array in main RAM, outside this overlay image. Its words
 * 0, 4 and 8 are the coordinate triplet indexed here; the address is bound by
 * this target's generated undefined_syms_auto.txt, so no target-map row is
 * added for it. */
extern BmagicMagic05803PartyActionRow D_80145EC4[]; /* @source 0x80145EC4 @kind table */

/* Enemy action record array in main RAM, outside this overlay image, indexed at
 * slot minus three so its rows continue the party slots; bound through the same
 * generated file. */
extern BmagicMagic05803EnemyActionRow D_801EB664[]; /* @source 0x801EB664 @kind table */

/* Scratchpad work-object pointer cell at 0x1F800044, bound by this target's own
 * map. This target's other lifts, all measured exact when this seed was written
 * and each reading the same cell once per access group, are
 * clearWorkBytesBmagicMagic05803_801EFE9C,
 * recordWithinWorkPositionRadiusBmagicMagic05803_801EFDD4 and
 * seedWorkFromRecordCopyByte8AndWordTripletSetByteATo16AndWordCTo8ThenAdvanceWorkByte2BmagicMagic05803_801EF008.
 * The original therefore re-materializes the cell address and reloads the work
 * pointer before each group. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EF454
 * @behavior Overlay step body, word 6 of the nine-word step table the payload
 *           header holds at 0x801EEC04, dispatched between func_801EF338 and
 *           func_801EF5D4. It decrements work byte 0x09 and returns while the
 *           countdown is non-zero; when it reaches zero it seeds the work
 *           coordinate triplet from a battler action record: work byte 0x0B is
 *           the battler slot, and while it is below 3 the row comes from the
 *           0x140-byte party array at 0x80145EC4 at that slot, otherwise from
 *           the 0x118-byte enemy array at 0x801EB664 at slot minus three. The
 *           row words 0, 4 and 8 land in work words 0x34, 0x38 and 0x3C. It then
 *           sets work byte 0x0A to 0x10, the work words 0x0C and 0x18 to 0x10,
 *           and advances work byte 0x02 twice, the second increment reading the
 *           work pointer cell again because the increment store may alias it. The original re-reads work byte 0x0B before
 *           each record load and the cell before each access group, because a
 *           store through the loaded pointer may alias the cell. Leaf with no
 *           frame; it takes no arguments, returns nothing and reads or writes no
 *           other state.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementWorkByte9ThenOnZeroSeedWorkCoordTripletByBattlerSlotSetByteATo16WordsCAnd18To16AdvanceWorkByte2TwiceBmagicMagic05803_801EF454(void) {
  u8 *work;
  u8 *motion;
  u8 *next;

  if (--g_battle_work[0x09] != 0) {
    return;
  }
  motion = g_battle_work;
  if (motion[0x0B] < 3) {
    *(u32 *)(motion + 0x34) = D_80145EC4[motion[0x0B]].coord_00;
    *(u32 *)(motion + 0x38) = D_80145EC4[motion[0x0B]].coord_04;
    *(u32 *)(motion + 0x3C) = D_80145EC4[motion[0x0B]].coord_08;
  } else {
    *(u32 *)(motion + 0x34) = D_801EB664[motion[0x0B] - 3].coord_00;
    *(u32 *)(motion + 0x38) = D_801EB664[motion[0x0B] - 3].coord_04;
    *(u32 *)(motion + 0x3C) = D_801EB664[motion[0x0B] - 3].coord_08;
  }
  work = g_battle_work;
  work[0x0A] = 0x10;
  next = g_battle_work;
  *(u32 *)(work + 0x0C) = 0x10;
  *(u32 *)(work + 0x18) = 0x10;
  next[0x02]++;
  g_battle_work[0x02]++;
}
