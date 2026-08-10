#include "bof3/battle/battle03_internal.h"

/* @behavior initializes the deferred local/enemy halfword countdown from the
 * current primary value when bit `0x80` is set.
 * @source 0x801DCEF8
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void initDeferredBattlerCountdown(u32 arg0) {
  arg0 &= 0xffu;
  if (arg0 < 3u) {
    if ((D_80145E90[arg0].unk_80 & 0x80u) != 0u) {
      D_80145E90[arg0].unk_11c = (D_80145E90[arg0].unk_88 + 5) / 10;
    }
  } else {
    if ((D_801EB630[arg0 - 3u].unk_82 & 0x80u) != 0u) {
      D_801EB630[arg0 - 3u].unk_f8 = (D_801EB630[arg0 - 3u].unk_94 + 5) / 10;
    }
  }
}
