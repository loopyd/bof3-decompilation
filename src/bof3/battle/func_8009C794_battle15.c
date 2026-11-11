#include "bof3/battle/battle15_internal.h"

u8 testEntryBitE1(volatile u8 *entry, s32 bit_index);

/* @source 0x8009C794
 * @behavior Reports whether the local panel slots of a battler record other
 *   than `slot` are usable. For each of the four slot indexes outside `slot`,
 *   the class-rule byte at 0x800E4048 + class*0x88 + slot*0x10 + 0x34 must not
 *   read the 0x63 sentinel and the record's slot flag bit in byte +0xE1 must be
 *   set. `index` selects the 0x118-byte battler record; its class byte comes
 *   from D_801EB710 + index*0x118. Returns 0 when any other slot fails either
 *   test, 1 when every other slot passes.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * The class-rule base stays an explicit fixed address so it reaches the
 * assembler as a full 32-bit constant (`lui`/`ori`), and the scaled slot byte is
 * carried in its own pointer local: the original accumulates it into the rule
 * pointer (`addu v0,v0,v1`) instead of adding the rule base to the scaled index,
 * which is what the indexed form emits.
 */
u8 func_8009C794(u8 slot, u8 index) {
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
