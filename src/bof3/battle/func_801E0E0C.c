#include "bof3/battle/battle03_internal.h"

/* @behavior Reports whether the first three D_80145E90 local work records are all
 * strictly ready: an inactive record is skipped, and an active record fails the
 * predicate when its +0x80 halfword carries 0x4004, its +0x124 word carries 4, its
 * +0x128 word carries 2, or -- while +0x80 carries 0x4000 -- its identity bytes
 * +0x86/+0x87 read 0x18 or its +0x85 byte reads 0x43.
 * @source 0x801E0E0C
 * @status exact
 * @match 100.00
 * @residual none
 */
u8 func_801E0E0C(void) {
  u8 index;

  index = 0u;
  while (index < 3u) {
    if ((D_80145E90[index].flags_00 & 1u) != 0u) {
      if ((D_80145E90[index].unk_80 & 0x4004u) == 0u) {
        return 0u;
      }
      if ((D_80145E90[index].unk_124 & 4u) != 0u) {
        return 0u;
      }
      if ((D_80145E90[index].unk_128 & 2u) != 0u) {
        return 0u;
      }
      if ((D_80145E90[index].unk_80 & 0x4000u) != 0u) {
        if (D_80145E90[index].unk_86 == 0x18u) {
          return 0u;
        }
        if (D_80145E90[index].unk_87 == 0x18u) {
          return 0u;
        }
        if (D_80145E90[index].unk_85 == 0x43u) {
          return 0u;
        }
      }
    }
    index += 1u;
  }
  return 1u;
}
