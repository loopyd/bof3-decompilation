#include "bof3/battle/battle03_internal.h"

/* @source 0x801DB844
 * @behavior Reports whether one battler slot is a strict candidate for the
 * actor phase of the threshold-marking pass. Slots below three index the local
 * work records and slots three and above the enemy records; either kind must
 * have its active bit 1 set, must not carry its side rejection mask (0x4964
 * local, 0x4164 enemy), must satisfy the 0x801463CE-gated +0x10 bit of its flag
 * word and must clear the extra flag bits (0x14001 local, 0x4000 enemy). The
 * slot is then rejected when the global actor-phase byte at 0x80146324 already
 * equals the slot's own side code (2 local, 1 enemy) or 3.
 * @status exact
 * @match 100.00
 * @residual none
 * Live audit: 104/104 instructions, 416 bytes, live byte match. The 0x80145FB8
 * and 0x801EB734 alias arrays name the +0x128/+0x104 flag words of the local and
 * enemy record arrays; the byte index itself stays an uncast u8 so the original
 * recomputes `andi ... ,0xff` per use instead of widening the index expression.
 */
u8 func_801DB844(u32 arg0) {
  u8  index;
  u8  phase;
  u32 state;

  index = (u8)arg0;
  if (index < 3u) {
    if ((D_80145E90[index].flags_00 & 1u) == 0u) {
      return 0u;
    }
    if ((D_80145E90[index].unk_80 & 0x4964u) != 0u) {
      return 0u;
    }
    if ((BATTLE_GLOBAL_BYTE_63CE != 0u) &&
        ((D_80145FB8[index].flags_00 & 0x10u) == 0u)) {
      return 0u;
    }
    if ((D_80145FB8[index].flags_00 & 0x14001u) != 0u) {
      return 0u;
    }
    state = BATTLE_GLOBAL_BYTE_6324;
    phase = 2u;
  } else {
    index = (u8)(arg0 - 3u);
    if ((D_801EB630[index].unk_00 & 1u) == 0u) {
      return 0u;
    }
    if ((D_801EB630[index].unk_82 & 0x4164u) != 0u) {
      return 0u;
    }
    if ((BATTLE_GLOBAL_BYTE_63CE != 0u) &&
        ((D_801EB734[index].word_00 & 0x10u) == 0u)) {
      return 0u;
    }
    if ((D_801EB734[index].word_00 & 0x4000u) != 0u) {
      return 0u;
    }
    state = BATTLE_GLOBAL_BYTE_6324;
    phase = 1u;
  }

  if ((state == phase) || (state == 3u)) {
    return 0u;
  }
  return 1u;
}
