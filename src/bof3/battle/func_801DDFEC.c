#include "bof3/battle/battle03_internal.h"

/* @source 0x801DDFEC
 * @behavior scans the ranked owner list from the D_80146322 cursor up to the
 * volatile D_80146323 count for the local battler named by the low byte of
 * arg0 and, when that battler's D_80145E90 record sits in state 5 with a clear
 * packed high byte, advances its counter through advanceCounterStorePacked.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801DDFEC(u32 arg0) {
  u32 index;
  u32 battler;
  u8  key;

  if ((arg0 & 0xffu) >= 3u) {
    return;
  }
  index = BATTLE_GLOBAL_BYTE_6322 - 1u;
  while ((index & 0xffu) < BATTLE_GLOBAL_BYTE_6323) {
    /* The QI key is refreshed at the top of the loop body: the original keeps
     * the raw argument in $a0 and recomputes the low-byte mask for each of the
     * guard and the loop compare, and reorg steals this one into the latch
     * delay slot. Folding it with the guard's expression instead leaves the
     * masked value in $a0 across the loop and drops that delay slot. */
    key = arg0;
    battler = (&D_8014630C)[index & 0xffu];
    if (battler == key) {
      if (D_80145E90[battler].unk_119 != 5u) {
        return;
      }
      if ((D_80145E90[battler].unk_11a >> 8) != 0u) {
        return;
      }
      advanceCounterStorePacked(D_80145E90[battler].unk_122,
                                D_80145E90[battler].unk_11a);
      return;
    }
    index += 1u;
  }
}
