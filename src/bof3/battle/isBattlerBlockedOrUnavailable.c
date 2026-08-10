#include "bof3/battle/battle03_internal.h"

/* @behavior reports whether one battler should be treated as blocked, taking the
 * global `0x10` suppression countdown into account before falling back to the
 * generic availability helper.
 * @source 0x801D64C4
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
u8 isBattlerBlockedOrUnavailable(u32 arg0) {
  u8  idx;
  u32 flags;

  idx = (u8)arg0;
  if (idx < 3u) {
    if (BATTLE_GLOBAL_BYTE_63CE != 0u) {
      flags = D_80145E90[idx].unk_128;
      if ((flags & 0x10u) == 0u) {
        return 1u;
      }
    }
  } else {
    if (BATTLE_GLOBAL_BYTE_63CE != 0u) {
      flags = D_801EB630[idx - 3u].unk_104;
      if ((flags & 0x10u) == 0u) {
        return 1u;
      }
    }
  }

  return func_801DB524((u8)arg0) != 0u;
}
