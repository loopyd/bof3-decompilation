#include "bof3/battle/battle03_internal.h"

/* @source 0x801DB9E4
 * @behavior Reports whether one battler slot is an eligible candidate for the
 * actor phase of the threshold-marking pass. Slots below three are local/party
 * records and slots three and above are enemy records; either kind must have
 * its active bit 1 set, must not carry its base rejection bits (0x4944 local,
 * 0x4144 enemy) and, when the global 0x801463CE gate is set, must have word
 * +0x128 (local) or +0x104 (enemy) bit 0x10 set. The slot is then rejected
 * when the global actor-phase byte at 0x80146324 already equals the slot's own
 * side code (2 local, 1 enemy) or 3.
 * @status exact
 * @match 100.00
 * @residual none
 * Live audit: 79/79 instructions, 316 bytes, live byte match. The actor-phase
 * byte is held in a word local: every rejection path stores the byte as read by
 * `lbu`, so the shared comparison compares the zero-extended load result
 * directly.
 */
u8 func_801DB9E4(u32 arg0) {
  u8  index;
  u8  phase;
  u32 state;

  index = (u8)arg0;
  if (index < 3u) {
    if ((D_80145E90[index].flags_00 & 1u) == 0u) {
      return 0u;
    }
    if ((D_80145E90[index].unk_80 & 0x4944u) != 0u) {
      return 0u;
    }
    if ((BATTLE_GLOBAL_BYTE_63CE != 0u) &&
        ((D_80145E90[index].unk_128 & 0x10u) == 0u)) {
      return 0u;
    }
    state = BATTLE_GLOBAL_BYTE_6324;
    phase = 2u;
  } else {
    if ((D_801EB630[(u8)(arg0 - 3u)].unk_00 & 1u) == 0u) {
      return 0u;
    }
    if ((D_801EB630[(u8)(arg0 - 3u)].unk_82 & 0x4144u) != 0u) {
      return 0u;
    }
    if ((BATTLE_GLOBAL_BYTE_63CE != 0u) &&
        ((D_801EB630[(u8)(arg0 - 3u)].unk_104 & 0x10u) == 0u)) {
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
