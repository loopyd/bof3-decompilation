#include "bof3/bof3.h"

extern u8 D_80143F03; /* @source 0x80143F03 @kind data */
extern u8 D_80143F06; /* @source 0x80143F06 @kind data */
extern u8 D_80144EA9; /* @source 0x80144EA9 @kind data */
extern u8 D_80144F28[]; /* @source 0x80144F28 @kind data */
extern u16 D_80146258; /* @source 0x80146258 @kind data */
extern u8 g_ScenarioProgress; /* @source 0x80146864 @kind data */
extern u8 D_801F4960[4][4]; /* @source 0x801F4960 @kind data */

/* @source 0x801A4BC0 */
extern void func_801A4BC0(s32 x, s32 y, u32 size);
/* @source 0x8015B5D4 */
extern s32 func_8015B5D4(void* bank, u8 bit_index);
/* @source 0x8015B5A8 */
extern void func_8015B5A8(void* bank, u8 bit_index);
/* @source 0x8015507C */
extern void func_8015507C(s16 x, s16 y);

/* @source 0x801F3A00
 * @behavior When byte 0x80143F03 is not 1, clears the low nibble of
 *           0x80144EA9 and queues x=0x41..0x44, y=3..9 with size 0xA4.
 *           When that byte is 6 and flag 0x27 in bank 0x80144F28 is clear,
 *           queues x=0x92..0x93 at y=0x1D with size 0 and y=0x1C with
 *           size 0x50. When byte 0x80143F06 is 6 and 0x80143F03 is 2,
 *           clears bit 0x1000 of halfword 0x80146258, clears flag 0x27,
 *           and clears scenario progress. When 0x80143F06 is 4, calls
 *           0x8015507C for nonzero entries of the four-by-four byte table
 *           at 0x801F4960, with coordinates x=0x94..0x97, y=0x20..0x23.
 * @status exact
 * @match 100.00
 * @residual none
 */
void updateUiBlocksAndClearFlag27BySharedModesWorld01Area04013_801F3A00(void) {
  u8 y;
  u8 x;
  u8* mode = &D_80144EA9;

  if (D_80143F03 != 1) {
    *mode &= 0xF0;
    for (y = 0; y < 7; y++) {
      func_801A4BC0(0x41, y + 3, 0xA4);
      func_801A4BC0(0x42, y + 3, 0xA4);
      func_801A4BC0(0x43, y + 3, 0xA4);
      func_801A4BC0(0x44, y + 3, 0xA4);
    }
  }
  if (D_80143F03 == 6 && func_8015B5D4(D_80144F28, 0x27) == 0) {
    func_801A4BC0(0x92, 0x1D, 0);
    func_801A4BC0(0x93, 0x1D, 0);
    func_801A4BC0(0x92, 0x1C, 0x50);
    func_801A4BC0(0x93, 0x1C, 0x50);
  }
  if (D_80143F06 == 6) {
    if (D_80143F03 == 2) {
      D_80146258 &= 0xEFFF;
      func_8015B5A8(D_80144F28, 0x27);
      g_ScenarioProgress = 0;
    }
  }
  if (D_80143F06 == 4) {
    for (y = 0; y < 4; y++) {
      for (x = 0; x < 4; x++) {
        if (D_801F4960[y][x] != 0) {
          func_8015507C(x + 0x94, y + 0x20);
        }
      }
    }
  }
}
