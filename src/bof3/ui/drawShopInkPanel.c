#include "bof3/ui/shop00_internal.h"

/* Tile-record streams of the shop ink panel, submitted to the main-exe
 * record-table renderer func_801AF390. Each stream is a run of 3-byte records:
 * `00 00 28 / 00 01 2B / 00 02 2D / 00 00 FF` at D_801E5CC0, `01 00 29 /
 * 01 02 2E / 00 00 FF` at D_801E5CCC, which is followed by three zero bytes of
 * padding, and `0A 00 2A / 0A 01 2C / 0A 02 2F / 00 00 FF` at D_801E5CD8. All
 * three are read-only payload data inside the rodata tail (the extracted label
 * T_801E5144 carries the segment), so no target-map row is added and the record
 * fields stay uninterpreted.
 * @source 0x801E5CC0
 * @kind table */
extern const u8 D_801E5CC0[];
/* @source 0x801E5CCC
 * @kind table */
extern const u8 D_801E5CCC[];
/* @source 0x801E5CD8
 * @kind table */
extern const u8 D_801E5CD8[];

/* Caption run of the shop ink panel: the payload holds the bytes `Ink` and a
 * terminator at this address inside the rodata tail, and this body is the only
 * reader of the string in the payload. It is handed to the main-exe text
 * emitter func_8014F800 as that call's fifth argument, the caption run the
 * emitter takes after the coordinates, the alignment argument and the mask.
 * @source 0x801E5D38
 * @kind data */
extern const u8 D_801E5D38[];

/* EMI-local format string of the ink row: the payload holds the three bytes
 * `%3d` and a terminator here, so the count is drawn right-aligned in a
 * three-character field. Not owned by the shared map and carried by the
 * generated undefined-symbol list, so no target-map row is added.
 * @source 0x801D0E84
 * @kind rodata */
extern const char D_801D0E84[];

/* Main-exe record-table renderer (executable space; lifted as
 * drawSpriteRecordTable in emi/etc/game/00) called with the panel origin, one
 * of this EMI's record streams and a zero flag. */
/* @source 0x801AF390 */
extern void func_801AF390(s16 arg0, s16 arg1, const u8* arg2, u8 arg3);

/* Text emitter of the main executable: the two signed coordinates, the
 * alignment argument, the mask and the caption run as the fifth argument, which
 * the original stores to the outgoing stack slot.
 * @source 0x8014F800 */
extern void func_8014F800(s16 arg0, s16 arg1, s32 arg2, u32 arg3, u32 arg4);

/* Command-code mapper of the main executable: it reports the mapped code of the
 * table it is given in the requested mode, and this body keeps only the low
 * halfword of that answer before formatting it.
 * @source 0x8016628C */
extern s32 func_8016628C(s32 table, s32 code, s32 mode);

/* @source 0x801E1E80
 * @behavior shop ink panel of the panel task: paints the 0x55 by 0x10 shaded
 *           panel rectangle at (x + 3, y + 3) through the main-exe rectangle
 *           helper func_801AE3F0 with the constant 0 and the main-RAM
 *           CLUT-bank byte D_80144952 as the trailing argument pair, submits the
 *           caption D_801E5D38 at (x + 4, y + 4) through the shared text
 *           emitter func_8014F800 with a zero alignment argument and the mask
 *           0xFF, formats the low halfword
 *           of the command-code mapper func_8016628C answer for table 0 and
 *           code 0x58 through sprintf into the shared text buffer D_80145AD4
 *           with this EMI's "%3d" format string D_801D0E84, draws that buffer at
 *           (x + 0x34, y + 4) through the shared text drawer func_8014FF0C,
 *           then submits three tile-record streams through func_801AF390: the
 *           first stream D_801E5CC0 once at (x, y), the second stream
 *           D_801E5CCC nine times from x to x + 64 in steps of 8 pixels along
 *           the same y, and the third stream D_801E5CD8 once at (x, y). Both
 *           parameters are word-sized: the body narrows each one to the
 *           unsigned halfword the panel arithmetic works with, so every use
 *           truncates the coordinate it reads, with `andi 0xFFFF` for the
 *           unsigned pair of the rectangle call and a sign-extending shift for
 *           each signed coordinate.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawShopInkPanel(s32 x, s32 y) {
  u16 ux = x;
  u16 uy = y;
  u8 i;

  func_801AE3F0((u16)(ux + 3), (u16)(uy + 3), 0x55, 0x10, 0, D_80144952);
  func_8014F800((s16)(ux + 4), (s16)(uy + 4), 0, 0xFF, (u32)D_801E5D38);
  sprintf((char*)D_80145AD4, D_801D0E84, (u16)func_8016628C(0, 0x58, 0));
  func_8014FF0C((s16)(ux + 0x34), (s16)(uy + 4), 0, D_80145AD4);
  func_801AF390((s16)ux, (s16)uy, D_801E5CC0, 0);
  for (i = 0; i < 9; i++) {
    func_801AF390((s16)(ux + i * 8), (s16)uy, D_801E5CCC, 0);
  }
  func_801AF390((s16)ux, (s16)uy, D_801E5CD8, 0);
}
