#include "bof3/battle/battle15_internal.h"

/* The 0x74-byte head of an owner work record, copied as one block. The
 * original copies it in four-word chunks with a trailing word, which is the
 * block-move codegen this type produces. */
typedef struct BattleOwnerHead {
  u32 words[0x1C];
  u32 tail;
} BattleOwnerHead;

extern void func_800AC218(u8 index, u8 class_id); /* @source 0x800AC218 */
extern void func_800AD0A8(u8 index, u16 class_id); /* @source 0x800AD0A8 */

/**
 * @source 0x800AC01C
 * @behavior Claims the owner work record of party slot `slot`: when
 *   findOwnerRecord returns no record the body returns untouched, otherwise it
 *   copies that record head (0x74 bytes) into the 0x118-byte local panel record
 *   selected by `index` at D_801EB630 + index*0x118, publishes that record as
 *   the active work pointer 0x1F800044, resets the work state it inherited
 *   (byte 1, the 2..4 triple, byte 0x29 to 4, byte 5 to index+3, the 0x5C..0x5F
 *   quad, the six words 0x0C..0x20, bytes 6, 7, 0x2B and 0x48, and byte 8 to
 *   the battle mode byte D_801462EC toggled by 2), initialises the record from
 *   its class template through func_800AC218, applies the class rule pair
 *   through func_800AD0A8, clears band byte 0x7F of the same record, raises
 *   flag 0x40 of the owner record leading byte and advances the initialised
 *   record counter D_801462F2.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Shape evidence: the reset half of this body is the record reset that the
 * battle/03 sibling 0x801DEF0C spells with one pointer local per store group
 * (measured partial there), and the descending zero triple is the one the exact
 * battle/03 0x801DF3F8 carried. The block copy is a plain struct assignment of
 * the 0x74-byte head: a byte-offset __builtin_memcpy of the same size unrolled
 * the whole copy instead of the original four-word loop plus trailing word
 * (measured 80/127 at 620 bytes), and a hand-written four-word loop left the
 * loop bump and the trailing word scheduled one slot away (measured 122/127 at
 * 504 bytes). Levers: analysis data-candidates reported no data span in this
 * target, a scan of every shipped payload for the whole 0x1FC span finds only
 * this payload, and the range starts with a prologue and ends with jr $ra, so
 * data, carve and clone are all declined.
 */
void claimOwnerSlotInitLocalPanelRecordResetWorkAndAdvanceCount(u8 slot, u8 index,
                                                               u8 class_id) {
  u8 *rec;
  u8 *band;
  u8 *work;
  u8 *work1;
  u8 *work2;
  u8 *work3;
  u8 *work4;
  u8 *work5;
  u8 *work6;
  u8 *work7;
  u8 *work8;
  u8 *work9;
  u8 *work10;

  rec = findOwnerRecord(slot);
  if (rec == NULL) {
    return;
  }

  band = (u8 *)D_801EB630 + (u32)index * 0x118u;
  *(BattleOwnerHead *)band = *(BattleOwnerHead *)rec;

  work = (u8 *)D_801EB630 + (u32)index * 0x118u;
  g_battle_work = work;
  work[1] = 0;
  work1 = g_battle_work;
  work1[4] = 0;
  work1[3] = 0;
  work1[2] = 0;
  work2 = g_battle_work;
  work2[0x29] = 4;
  work3 = g_battle_work;
  work3[5] = index + 3;
  work4 = g_battle_work;
  work4[0x5F] = 0;
  work4[0x5E] = 0;
  work4[0x5D] = 0;
  work5 = g_battle_work;
  *(u32 *)(work4 + 0x14) = 0;
  *(u32 *)(work4 + 0x10) = 0;
  *(u32 *)(work4 + 0x0C) = 0;
  *(u32 *)(work4 + 0x20) = 0;
  *(u32 *)(work4 + 0x1C) = 0;
  *(u32 *)(work4 + 0x18) = 0;
  work5[0x5C] = 0;
  work6 = g_battle_work;
  work6[6] = 0;
  work7 = g_battle_work;
  work7[7] = 0;
  work8 = g_battle_work;
  work8[0x2B] = 0;
  work9 = g_battle_work;
  work9[8] = D_801462EC ^ 2;
  work10 = g_battle_work;
  work10[0x48] = 0;
  func_800AC218(index, class_id);
  func_800AD0A8(index, class_id);
  D_801EB6AF[(u32)index * 0x118u] = 0;
  rec[0] |= 0x40;
  D_801462F2 += 1;
}
