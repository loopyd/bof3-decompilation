#include "internal.h"

/* does: finds the current local battler inside the ranked owner list and, if it
 * is in the expected intermediate state, advances the paired counter table.
 * @source: 0x801ddfec FUN_801ddfec
 */
void func_801ddfec(u32 arg0) {
  u32 index;
  u32 battler;

  if ((arg0 & 0xffu) < 3u) {
    index = BOF3_BATTLE_GLOBAL_BYTE_6322 - 1u;
    while ((index & 0xffu) < BOF3_BATTLE_GLOBAL_BYTE_6323) {
      battler = BOF3_BATTLE_GLOBAL_BYTE_630C(index);
      if (battler == (arg0 & 0xffu)) {
        if (BOF3_BATTLE_LOCAL_BYTE_119(
                &BOF3_BATTLE_LOCAL_WORK_ARRAY[battler]) != 5u) {
          return;
        }
        index =
            BOF3_BATTLE_LOCAL_HALF_11A(&BOF3_BATTLE_LOCAL_WORK_ARRAY[battler]);
        if ((index >> 8) != 0u) {
          return;
        }
        func_801dde7c(
            BOF3_BATTLE_LOCAL_BYTE_122(&BOF3_BATTLE_LOCAL_WORK_ARRAY[battler]),
            index);
        return;
      }
      index += 1u;
    }
  }
}
