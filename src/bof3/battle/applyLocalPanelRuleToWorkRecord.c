#include "bof3/battle/battle15_internal.h"

void func_8009CCBC(u8 *obj, u8 index, u8 value); /* @source 0x8009CCBC */
void func_8009CF80(u8 *obj, u8 index, u8 value); /* @source 0x8009CF80 */

/* Selection byte written by rule operation 7. */
extern u8 D_801463D7; /* @source 0x801463D7 @kind unknown */

/* @source 0x8009C994
 * @behavior Applies one 16-byte local panel rule to a battle work record. The
 *   rule byte +0x01 selects the operation: 1 stores the rule byte +0x02 into
 *   the work halfword +0x80; 2 publishes the work record at 0x1F800044 and runs
 *   func_800A36F0 with the work byte +0x05 and 0xFFFF, then func_800A31E0 with
 *   the published record byte +0x05 and the rule byte +0x02 or-ed with the work
 *   byte +0x82; 3 and 4 walk the eight high bits of the rule byte +0x02 and call
 *   func_8009CCBC or func_8009CF80 for each set bit with the work record, the
 *   bit index and the rule byte +0x03; 5 stores the rule byte +0x03 at work
 *   +0x9A for mask 2 and at work +0x9E for mask 1; 6 scales the work halfwords
 *   +0x86 and +0x84 by the rule byte +0x03 through times 100 over 1000 and
 *   clamps each result to 0xFFFF for masks 2 and 1; 7 stores the rule byte +0x02 at
 *   0x801463D7; 0 and 8 or more change no work field. Every rule then stores
 *   the rule byte +0x04 at work +0x7E, copies the eight rule bytes +0x08..+0x0F
 *   to work +0x8C..+0x93, calls func_800A9BD8 with the work byte +0x05 and,
 *   when the work halfword +0x94, the signed rule halfword +0x06 and an entry
 *   count below eight all hold, appends a local panel entry whose owner index is
 *   the work byte +0x05 minus three and whose panel id is the rule halfword
 *   +0x06.
 * @status exact
 * @match 100.00
 * @residual none
 */
void applyLocalPanelRuleToWorkRecord(u8 *work, u8 *rule) {
  u8 i;
  u8 bits;
  u8 *saved;
  u32 scaled;
  s32 amount;

  switch (rule[1]) {
  case 0:
    break;
  case 1:
    *(u16 *)(work + 0x80) = rule[2];
    break;
  case 2:
    saved = g_battle_work;
    g_battle_work = work;
    func_800A36F0(work[5], 0xFFFF);
    func_800A31E0(g_battle_work[5], work[0x82] | rule[2]);
    g_battle_work = saved;
    break;
  case 3:
    bits = rule[2];
    for (i = 0; i < 8; i++) {
      if (bits & 0x80) {
        func_8009CCBC(work, i, rule[3]);
      }
      bits <<= 1;
    }
    break;
  case 4:
    bits = rule[2];
    for (i = 0; i < 8; i++) {
      if (bits & 0x80) {
        func_8009CF80(work, i, rule[3]);
      }
      bits <<= 1;
    }
    break;
  case 5:
    if (rule[2] & 2) {
      work[0x9A] = rule[3];
    }
    if (rule[2] & 1) {
      work[0x9E] = rule[3];
    }
    break;
  case 6:
    if (rule[2] & 2) {
      scaled = *(u16 *)(work + 0x86) * (amount = rule[3] * 100) / 1000;
      if (scaled <= 0xFFFF) {
        *(u16 *)(work + 0x86) = scaled;
      } else {
        *(u16 *)(work + 0x86) = 0xFFFF;
      }
    }
    if (rule[2] & 1) {
      scaled = *(u16 *)(work + 0x84) * (amount = rule[3] * 100) / 1000;
      if (scaled <= 0xFFFF) {
        *(u16 *)(work + 0x84) = scaled;
      } else {
        *(u16 *)(work + 0x84) = 0xFFFF;
      }
    }
    break;
  case 7:
    D_801463D7 = rule[2];
    break;
  default:
    break;
  }

  work[0x7E] = rule[4];
  /* The additive spelling keeps each copy address as base plus masked index:
   * the array form `work[0x8C + i]` folds into index-plus-base and swaps the
   * `addu` operands against the original. */
  for (i = 0; i < 8; i++) {
    *(u8 *)(0x8C + (work + i)) = *(u8 *)(8 + (rule + i));
  }
  func_800A9BD8(work[5]);
  if (*(u16 *)(work + 0x94) != 0 && *(s16 *)(rule + 6) != 0 &&
      D_801EB5A8 < 8) {
    D_801ED9B0[D_801EB5A8].owner_index = work[5] - 3;
    D_801ED9B0[D_801EB5A8].panel_id = *(u16 *)(rule + 6);
    D_801EB5A8++;
  }
}
