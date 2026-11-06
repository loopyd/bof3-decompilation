#include "bof3/bof3.h"

/* @source 0x801448EB @kind unknown */
extern u8 D_801448EB;
/* @source 0x801448EC @kind unknown */
extern u8 D_801448EC;
/* @source 0x80145E98 @kind unknown */
extern u8 D_80145E98;
/* @source 0x80146867 @kind unknown */
extern u8 D_80146867;

u32 func_80166140(u32 arg0);

/* @source 0x801F2C98
 * @behavior Overlay gate that starts the shared state entry 0xD at progress 0.
 *           It rejects a second argument above 0x3A0000, a first argument whose
 *           high half is outside 0x30..0x32, a record-0 mode byte D_80145E98
 *           outside 0..2 and a nonzero shared selector func_80166140(5); on
 *           pass it stores 1 into the mode flag D_80146867, 0xD into the shared
 *           state entry D_801448EB and 0 into the progress byte D_801448EC, and
 *           returns 1. Every rejected path returns 0.
 * @status exact
 * @match 100.00
 * @residual none
 */
s32 startStateEntry0DProgress0WhenMapRangeAndModeGateWorld02Area09713_801F2C98(s32 arg0, s32 arg1) {
  u8 mode;

  mode = D_80145E98;
  if (arg1 > 0x3A0000) {
    return 0;
  }
  if ((u32)((arg0 >> 16) - 0x30) >= 3u) {
    return 0;
  }
  if (!((u32)mode < 2u || mode == 2)) {
    return 0;
  }
  if (func_80166140(5) != 0) {
    return 0;
  }
  D_80146867 = 1;
  D_801448EB = 0xD;
  D_801448EC = 0;
  return 1;
}
