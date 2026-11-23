#include "bof3/battle/battle15_internal.h"

u8 testEntryBitE1(volatile u8 *entry, s32 bit_index);

/* @source 0x8009C794
 * @behavior Returns 1 when every slot index from 0 through 3 other than slot
 *   passes both checks. The rule byte at 0x800E4048 + record[0xE0]*0x88 +
 *   slotIndex*0x10 + 0x34 must differ from 0x63, and the corresponding bit
 *   of record byte +0xE1 must be set. Returns 0 at the first failed check.
 *   index selects the 0x118-byte record at D_801EB630; arguments are not
 *   checked against valid record or slot domains.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * The explicit 32-bit rule base and separate scaled-slot pointer preserve
 * the original lui/ori materialization and pointer-addition order.
 */
u8 allOtherHaveRule(u8 slot, u8 index) {
  u8 i;
  u32 offset;
  u32 class_id;
  u8 *rule;
  u8 *slot_rule;

  i = 0;
  offset = (u32)index * 0x118u;
  for (; i < 4; i++) {
    if (i != slot) {
      class_id = D_801EB710[offset];
      rule = PSX_PTR(u8, 0x800E4048u) + class_id * 0x88u;
      slot_rule = rule + (u32)i * 0x10u;
      if (slot_rule[0x34] == 0x63u) {
        return 0;
      }
      if (testEntryBitE1((u8 *)D_801EB630 + offset, i) == 0) {
        return 0;
      }
    }
  }
  return 1;
}
