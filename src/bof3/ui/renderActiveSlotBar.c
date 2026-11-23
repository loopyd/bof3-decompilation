#include "bof3/bof3.h"

/* Shaded-rectangle filler of the resident executable: paints the rectangle at
 * (x, y) with the given size, shade byte and clut byte. */
extern void func_801AE3F0(u16 x, u16 y, u32 width, u32 height, u32 shade,
                          u32 clut); /* @source 0x801AE3F0 */

/* Window-tile emitter of this overlay: draws piece of the six-byte sprite rect
 * table at 0x801DAE88 at (x, y). */
extern void func_801D875C(s16 x, s16 y, u8 piece);

extern void drawWindowFrame(s16 x, s16 y);
extern u8 countActiveRecords_commu02_08(void);

/* Shared clut byte of the panels the resident executable paints. */
extern u8 D_80144952; /* @source 0x80144952 @kind data */

/* @source 0x801D8058
 * @behavior Draws the active-record slot bar: paints the rectangle at (x, y)
 *           0xAC wide by 0x1A high through func_801AE3F0 with shade 0x80 and
 *           the main-RAM clut-bank byte 0x80144952, frames it through
 *           drawWindowFrame at the same origin, then draws the eight-wide
 *           sixteen-high slot tile, piece 1 of the six-byte sprite rect table
 *           at 0x801DAE88, at (x + 7, y + 9) once per active record that
 *           countActiveRecords_commu02_08 counts, leaving the bar empty when
 *           that count is zero. Every coordinate is truncated to sixteen bits
 *           per primitive. No state is written.
 * @status exact
 * @match 100.00
 * @residual none
 */
void renderActiveSlotBar(s16 x, s16 y) {
  s32 i;
  s32 offset;
  u8 count;

  func_801AE3F0((u16)x, (u16)y, 0xAC, 0x1A, 0x80, D_80144952);
  drawWindowFrame((s16)x, (s16)y);
  count = countActiveRecords_commu02_08();
  for (i = 0; i < count; i++) {
    offset = i * 8 + 7;
    func_801D875C((s16)(x + offset), (s16)(y + 9), 1);
  }
}
