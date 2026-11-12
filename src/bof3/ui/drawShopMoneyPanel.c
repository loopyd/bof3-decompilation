#include "bof3/ui/shop00_internal.h"

/* Tile-record streams of the shop money panel, submitted to the main-exe
 * record-table renderer func_801AF390. Each stream is a run of 3-byte records:
 * `00 00 28 / 00 01 2B / 00 02 2D / 00 00 FF` at D_801E555C,
 * `01 00 29 / 01 02 2E / 00 00 FF` at D_801E5568, which is followed by three
 * zero bytes of padding, and `0C 00 2A / 0C 01 2C / 0C 02 2F / 00 00 FF` at
 * D_801E5574. All three are read-only payload data inside the rodata tail (the
 * extracted label T_801E5144 carries the segment), so no target-map row is
 * added and the record fields stay uninterpreted.
 * @source 0x801E555C
 * @kind table */
extern const u8 D_801E555C[];
/* @source 0x801E5568
 * @kind table */
extern const u8 D_801E5568[];
/* @source 0x801E5574
 * @kind table */
extern const u8 D_801E5574[];

/* Texture handle submitted as the fifth argument of the texture row emitter
 * func_8014F800; the payload holds the word pair 0x60 and 0x3E here and the
 * "Use" string follows at 0x801E58B8.
 * @source 0x801E58B0
 * @kind data */
extern const u8 D_801E58B0[];

/* EMI-local format string of the money row: the payload holds the three bytes
 * `%7d` and a terminator here, so the amount is drawn right-aligned in a
 * seven-character field. Not owned by the shared map and carried by the
 * generated undefined-symbol list, so no target-map row is added.
 * @source 0x801D0D08
 * @kind rodata */
extern const char D_801D0D08[];

/* Main-exe record-table renderer (executable space; lifted as
 * drawSpriteRecordTable in emi/etc/game/00) called with the panel origin, one
 * of this EMI's record streams and a zero flag. */
/* @source 0x801AF390 */
extern void func_801AF390(s16 arg0, s16 arg1, const u8* arg2, u8 arg3);

/* @source 0x801DA2F4
 * @behavior shop money panel of the panel task: paints the 0x65 by 0x10 shaded
 *           panel rectangle at (x + 3, y + 3) through the main-exe rectangle
 *           helper func_801AE3F0 with the constant 0 and the main-RAM
 *           CLUT-bank byte D_80144952 as the trailing argument pair, formats
 *           the s32 amount through sprintf into the shared text buffer
 *           D_80145AD4 with this EMI's "%7d" format string D_801D0D08, draws
 *           that buffer at (x + 0x14, y + 4) through the shared text drawer
 *           func_8014FF0C, submits the texture handle D_801E58B0 at
 *           (x + 0x4C, y + 4) through func_8014F800 with the constant 0xFF as
 *           the fourth argument, then submits three tile-record streams
 *           through func_801AF390: the first stream D_801E555C once at (x, y),
 *           the second stream D_801E5568 eleven times from x to x + 80 in steps
 *           of 8 pixels along the same y, and the third stream D_801E5574 once
 *           at (x, y). The third argument is never read by the body.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawShopMoneyPanel(u16 x, u16 y, s32 arg2, s32 amount) {
  u8 i;

  func_801AE3F0((u16)(x + 3), (u16)(y + 3), 0x65, 0x10, 0, D_80144952);
  sprintf((char*)D_80145AD4, D_801D0D08, amount);
  func_8014FF0C((s16)(x + 0x14), (s16)(y + 4), 0, D_80145AD4);
  func_8014F800((s16)(x + 0x4C), (s16)(y + 4), 0, 0xFF, (u32)D_801E58B0);
  func_801AF390((s16)x, (s16)y, D_801E555C, 0);
  for (i = 0; i < 11; i++) {
    func_801AF390((s16)(x + i * 8), (s16)y, D_801E5568, 0);
  }
  func_801AF390((s16)x, (s16)y, D_801E5574, 0);
}
