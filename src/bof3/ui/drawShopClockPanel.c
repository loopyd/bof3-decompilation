#include "bof3/ui/shop00_internal.h"

/* Tile-record streams of the shop clock panel, submitted to the main-exe
 * record-table renderer func_801AF390. Each stream is a run of 3-byte records:
 * `00 00 30 / 00 02 2D / 01 02 2E / 02 02 2E / 03 02 2E / 00 00 FF` at
 * D_801E5580, `0C 00 2A / 0C 01 2C / 0C 02 2F / 00 00 FF` at D_801E5574, and
 * D_801E5568, which this panel shares with the shop money panel. All three are
 * read-only payload data inside the rodata tail (the extracted label T_801E5144
 * carries the segment), so no target-map row is added and the record fields
 * stay uninterpreted.
 * @source 0x801E5580
 * @kind table */
extern const u8 D_801E5580[];
/* @source 0x801E5574
 * @kind table */
extern const u8 D_801E5574[];
/* @source 0x801E5568
 * @kind table */
extern const u8 D_801E5568[];

/* Payload word 0x3E at 0x801E58B4, handed to the main-exe text drawer
 * func_8014FC90 as its last argument: the word sits one 4-byte cell after the
 * 0x60 handle drawShopMoneyPanel submits through func_8014F800, and the
 * "Use"/"Sort" caption strings of the command row follow it. The role of the
 * cell beyond being that call's data pointer is not recovered.
 * @source 0x801E58B4
 * @kind data */
extern const u8 D_801E58B4[];

/* EMI-local format string of the two clock fields: the payload holds the four
 * bytes `%02d` and a terminator here, so each field is drawn as a zero-padded
 * two-digit number. Not owned by the shared map and carried by the generated
 * undefined-symbol list, so no target-map row is added.
 * @source 0x801D0DE0
 * @kind rodata */
extern const char D_801D0DE0[];

/* Main-exe record-table renderer (executable space; lifted as
 * drawSpriteRecordTable in emi/etc/game/00) called with the panel origin, one
 * of this EMI's record streams and a zero flag. */
/* @source 0x801AF390 */
extern void func_801AF390(s16 arg0, s16 arg1, const u8* arg2, u8 arg3);

/* Main-exe text drawer (executable space, unlifted): the battle targets submit
 * a record pointer with the constant 8 as the fourth argument, while this
 * target submits the payload cell D_801E58B4 with 0xFF. The widths below are
 * the widths observed at that battle call site, not a recovered contract. */
/* @source 0x8014FC90 */
extern void func_8014FC90(s16 arg0, s16 arg1, s32 arg2, u32 arg3,
                          const u8* arg4);

/* @source 0x801DA47C
 * @behavior shop clock panel of the panel task: paints the 0x55 by 0x10 shaded
 *           panel rectangle at (x + 3, y + 3) through the main-exe rectangle
 *           helper func_801AE3F0 with the constant 0 and the main-RAM
 *           CLUT-bank byte D_80144952 as the trailing argument pair, then
 *           formats the front-end play-time clock cells through this EMI's
 *           "%02d" string D_801D0DE0 into the shared text buffer D_80145AD4 and
 *           draws each field through the shared text drawer func_8014FF0C: the
 *           hour byte D_80144FC0 at (x + 0x21, y + 4) and the minute byte
 *           D_80144FC1 at (x + 0x3C, y + 4). While the sub-second byte
 *           D_80144FC3 reads below 0xF it submits the payload cell D_801E58B4
 *           twice through the main-exe text drawer func_8014FC90 at
 *           (x + 0x34, y + 3) and (x + 0x34, y + 7) with the constant 0xFF as
 *           the fourth argument, so the separator between the two fields blinks
 *           at half the clock tick rate. It then submits three tile-record
 *           streams through func_801AF390: the first stream D_801E5580 once at
 *           (x, y), the second stream D_801E5568 six times from x + 0x18 to
 *           x + 0x40 in steps of 8 pixels along the same y, and the third
 *           stream D_801E5574 once at (x - 0x10, y). The third and fourth
 *           arguments are never read by the body.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawShopClockPanel(u16 x, u16 y, s32 arg2, s32 arg3) {
  u8 i;

  func_801AE3F0((u16)(x + 3), (u16)(y + 3), 0x55, 0x10, 0, D_80144952);
  sprintf((char*)D_80145AD4, D_801D0DE0, D_80144FC0);
  func_8014FF0C((s16)(x + 0x21), (s16)(y + 4), 0, D_80145AD4);
  sprintf((char*)D_80145AD4, D_801D0DE0, D_80144FC1);
  func_8014FF0C((s16)(x + 0x3C), (s16)(y + 4), 0, D_80145AD4);
  if (D_80144FC3 < 0xF) {
    func_8014FC90((s16)(x + 0x34), (s16)(y + 3), 0, 0xFF, D_801E58B4);
    func_8014FC90((s16)(x + 0x34), (s16)(y + 7), 0, 0xFF, D_801E58B4);
  }
  func_801AF390((s16)x, (s16)y, D_801E5580, 0);
  for (i = 0; i < 6; i++) {
    func_801AF390((s16)(x + 0x18 + i * 8), (s16)y, D_801E5568, 0);
  }
  func_801AF390((s16)(x - 0x10), (s16)y, D_801E5574, 0);
}
