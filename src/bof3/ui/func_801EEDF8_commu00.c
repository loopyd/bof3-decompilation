#include "bof3/ui/commu00_internal.h"

/* @source 0x801EEDF8
 * @behavior Scans the type-45 notification table from the stored scan count
 * plus one through the notification limit, appending the first kind-4 and the
 * first kind-5 notification to the pending queue, then latches the scan limit
 * into the stored scan count.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801EEDF8(void) {
  u32 limit = D_80143F00;
  u32 count = D_801455C0;
  s32 index;
  u8 seen = 0;
  u32 kind;

  if (count >= limit) {
    return;
  }

  for (index = count + 1; index <= D_80143F00; index++) {
    kind = COMMU00_TYPE45_NOTIFICATION_TABLE[index];
    if (kind == 4) {
      if (!(seen & 1)) {
        s32 slot = D_80145E5D * 2;
        D_80145E60[slot] = kind;
        D_80145E5D = D_80145E5D + 1;
        seen |= 1;
      }
    } else if (kind == 5) {
      if (!(seen & 2)) {
        s32 slot = D_80145E5D * 2;
        D_80145E60[slot] = 5;
        D_80145E5D = D_80145E5D + 1;
        seen |= 2;
      }
    }
  }

  D_801455C0 = D_80143F00;
}
