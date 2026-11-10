#include "bof3/bof3.h"

/* @source 0x80144EA9 @kind data */
extern u8 D_80144EA9;

extern void func_801A4BC0(s32 x, s32 y, u32 size);

/* @source 0x801F2E78
 * @behavior Queues the shared UI block x = 0x41..0x44, y = 3..9 through
 *           func_801A4BC0. When the low nibble of 0x80144EA9 is 5, uses
 *           size 0 and returns 1; otherwise uses size 0xA4 and returns 0.
 *           Visits all four x positions for each successive y position.
 * @status exact
 * @match 100.00
 * @residual none
 */
s32 queueSharedUiBlock41To44x3To9ByModeWorld01Area04013_801F2E78(void) {
  u8 i;

  if ((D_80144EA9 & 0xF) == 5) {
    for (i = 0; i < 7; i++) {
      func_801A4BC0(0x41, i + 3, 0);
      func_801A4BC0(0x42, i + 3, 0);
      func_801A4BC0(0x43, i + 3, 0);
      func_801A4BC0(0x44, i + 3, 0);
    }
    return 1;
  } else {
    for (i = 0; i < 7; i++) {
      func_801A4BC0(0x41, i + 3, 0xA4);
      func_801A4BC0(0x42, i + 3, 0xA4);
      func_801A4BC0(0x43, i + 3, 0xA4);
      func_801A4BC0(0x44, i + 3, 0xA4);
    }
    return 0;
  }
}
