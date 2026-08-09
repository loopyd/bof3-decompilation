#include "bof3/battle/battle03_internal.h"

/* @behavior finds the current local battler inside the ranked owner list and, if it
 * is in the expected intermediate state, advances the paired counter table.
 * @source 0x801DDFEC
 * @status partial
 * @match 36.36
 * @residual non-exact live audit: 20/48 instructions; 192 original bytes versus 220 current.
 */
void func_801DDFEC(u32 arg0) {
  u32 index;
  u32 battler;

  if ((arg0 & 0xffu) < 3u) {
    index = BATTLE_GLOBAL_BYTE_6322 - 1u;
    while ((index & 0xffu) < BATTLE_GLOBAL_BYTE_6323) {
      battler = BATTLE_GLOBAL_BYTE_630C(index);
      if (battler == (arg0 & 0xffu)) {
        if (BATTLE_LOCAL_BYTE_119(&BATTLE_LOCAL_WORK_ARRAY[battler]) != 5u) {
          return;
        }
        index = BATTLE_LOCAL_HALF_11A(&BATTLE_LOCAL_WORK_ARRAY[battler]);
        if ((index >> 8) != 0u) {
          return;
        }
        advanceCounterStorePacked(BATTLE_LOCAL_BYTE_122(&BATTLE_LOCAL_WORK_ARRAY[battler]),
                      index);
        return;
      }
      index += 1u;
    }
  }
}
