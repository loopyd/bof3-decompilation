#include "internal.h"

/* @behavior clears paired local/enemy flag bits and associated countdown bytes for
 * the current battler, except under two specific global mode/kind combinations.
 * @source 0x801D4850
 */
void func_801D4850(void) {
  if ((D_80146375 != 4u) ||
      (D_801463C0 != 0x27u)) {
    if (D_80146374 < 3u) {
      if ((D_80145E90[D_80146374].unk_128 & 0x40u) != 0u) {
        D_80145E90[D_80146374].unk_138 = 0u;
        D_80145E90[D_80146374].unk_128 &= 0xffffffbfu;
      }
    } else {
      if ((D_801EB630[D_80146374 - 3u].unk_104 & 0x40u) !=
          0u) {
        D_801EB630[D_80146374 - 3u].unk_114 = 0u;
        D_801EB630[D_80146374 - 3u].unk_104 &= 0xffffffbfu;
      }
    }
  }

  if ((D_80146375 != 4u) ||
      (D_801463C0 != 0xa3u)) {
    if (D_80146374 < 3u) {
      if ((D_80145E90[D_80146374].unk_128 & 0x80u) != 0u) {
        D_80145E90[D_80146374].unk_139 = 0u;
        D_80145E90[D_80146374].unk_128 &= 0xffffff7fu;
      }
    } else {
      if ((D_801EB630[D_80146374 - 3u].unk_104 & 0x80u) !=
          0u) {
        D_801EB630[D_80146374 - 3u].unk_115 = 0u;
        D_801EB630[D_80146374 - 3u].unk_104 &= 0xffffff7fu;
      }
    }
  }
}
