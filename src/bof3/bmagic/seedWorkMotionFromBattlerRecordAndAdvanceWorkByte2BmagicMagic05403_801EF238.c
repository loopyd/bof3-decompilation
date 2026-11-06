#include "bof3/bof3.h"

/* Party action record row: the 0x140-byte rows the battle target indexes as
 * BattlePartyActionRecord (include/bof3/battle/battle15_internal.h), whose
 * first three words are the battler's coordinate triplet. That header is the
 * battle target's private declaration set, so this overlay binds the row shape
 * it reads, the same way the bmagic dispatcher siblings bind their table
 * shapes. The exact ui lifts publish the same two leading words at
 * 0x80145EC4/0x80145EC8 as the party coordinate pair. */
typedef struct BmagicMagic05403PartyActionRow {
  u32 coord_00;
  u32 coord_04;
  u32 coord_08;
  u8 unk_0C[0x140 - 0x0C];
} BmagicMagic05403PartyActionRow;

/* Enemy action record row: the 0x118-byte rows the battle target indexes as
 * BattleEnemyActionRecord, whose first three words are that record's
 * coordinate triplet. */
typedef struct BmagicMagic05403EnemyActionRow {
  u32 coord_00;
  u32 coord_04;
  u32 coord_08;
  u8 unk_0C[0x118 - 0x0C];
} BmagicMagic05403EnemyActionRow;

/* Party action record array in main RAM, outside this overlay's image. Its
 * words 0, 4 and 8 are the coordinate triplet indexed here; Splat binds the
 * address through this target's generated undefined_syms_auto.txt, which the
 * remaining asm sibling func_801EF534 still references, so no target-map row is
 * added here. */
extern BmagicMagic05403PartyActionRow D_80145EC4[]; /* @source 0x80145EC4 @kind table */

/* Enemy action record array in main RAM, outside this overlay's image, indexed
 * the same way; bound through the same generated file. */
extern BmagicMagic05403EnemyActionRow D_801EB664[]; /* @source 0x801EB664 @kind table */

/* Battle target selector byte in main RAM, outside this overlay's image. Its
 * 0x40 bit selects the enemy group here; the exact bmagic siblings
 * raiseBattleTargetFlag20ClearBytes9AThenAdvanceWorkByte1BmagicMagic09403_801EEF1C
 * and raiseBattleTargetFlag10WhenWorkByte9SetThenAdvanceWorkByte1BmagicMagic10003_801EED9C
 * pass the same byte to the shared battler-status helper. Bound through this
 * target's generated undefined_syms_auto.txt. */
extern u8 D_80146384; /* @source 0x80146384 @kind unknown */

/* Scratchpad work-object pointer cell. Declared as a named symbol rather than
 * through SPAD_PTR_SLOT so each access re-materialises the cell address with
 * its own lui, which is what the original does before every store group; the
 * bare constant address is instead hoisted once into a base register. The
 * exact sibling seedWorkObjectFromBattleRecordAndAdvanceWorkByte2BmagicMagic04500_801EF4A4
 * declares the same cell this way. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind unknown */

/* @source 0x801EF238
 * @behavior Seeds the scratchpad work object published at the pointer cell
 *           0x1F800044 for a battler motion. When bit 0x40 of the battle target
 *           byte 0x80146384 is set it copies the coordinate triplet (words 0, 4
 *           and 8) of the 0x118-byte-stride enemy action record array at
 *           0x801EB664 selected by work byte 0xB into work words 0x34, 0x38 and
 *           0x3C; otherwise it copies the same three words of the
 *           0x140-byte-stride party action record array at 0x80145EC4. It then
 *           sets the motion pair at 0x40 and 0x44 to 0x10000, byte 0x48 to 2,
 *           the motion velocity pair at 0xC and 0x10 to -0xC60 and 0x1080, the
 *           acceleration pair at 0x18 and 0x1C to 0x60 and -0x80, and advances
 *           work byte 2. The original re-reads work byte 0xB before each record
 *           load and re-reads the pointer cell before each of the three store
 *           groups, because a store through the loaded pointer may alias the
 *           cell. Leaf with no frame; it takes no arguments, returns nothing
 *           and reads or writes no other state.
 * @status exact
 * @match 100.00
 * @residual none
 */
void seedWorkMotionFromBattlerRecordAndAdvanceWorkByte2BmagicMagic05403_801EF238(void) {
  u8 *work;
  u8 *motion;
  u8 *velocity;

  if (D_80146384 & 0x40) {
    work = D_1F800044;
    *(u32 *)(work + 0x34) = D_801EB664[work[0xB]].coord_00;
    *(u32 *)(work + 0x38) = D_801EB664[work[0xB]].coord_04;
    *(u32 *)(work + 0x3C) = D_801EB664[work[0xB]].coord_08;
  } else {
    work = D_1F800044;
    *(u32 *)(work + 0x34) = D_80145EC4[work[0xB]].coord_00;
    *(u32 *)(work + 0x38) = D_80145EC4[work[0xB]].coord_04;
    *(u32 *)(work + 0x3C) = D_80145EC4[work[0xB]].coord_08;
  }
  motion = D_1F800044;
  *(u32 *)(motion + 0x40) = 0x10000u;
  *(u32 *)(motion + 0x44) = 0x10000u;
  motion[0x48] = 2;
  velocity = D_1F800044;
  *(u32 *)(velocity + 0xC) = -0xC60;
  *(u32 *)(velocity + 0x10) = 0x1080;
  *(u32 *)(velocity + 0x18) = 0x60;
  *(u32 *)(velocity + 0x1C) = -0x80;
  velocity[2]++;
}
