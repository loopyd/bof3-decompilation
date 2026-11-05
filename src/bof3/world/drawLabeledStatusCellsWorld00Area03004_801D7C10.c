#include "bof3/world/area03004_internal.h"

/* @source 0x801D7C10
 * @behavior draws the three AREA030 status-panel cells of one row at the
 * requested origin: bit i of the mask byte is the panel flag of cell i, the
 * 0x28 by 0x10 cell is drawn at (x + 0x30 * i, y) through func_801D7D68, and
 * the stored flag becomes a label shade that counts up by one when the whole
 * mask is zero; the function then submits the 4-character label runs of the
 * shared "GearDataRule" rodata string 0x801E21C0 at (x + 6, y + 4),
 * (x + 0x36, y + 4) and (x + 0x66, y + 4) through func_8014F800 with the
 * shade 7 - 7 * flag and the 4-token count.
 * @status partial
 * @match 97.67
 * @residual loop frame-base materialisation stays in place on the address
 * register instead of the original scratch/base pair
 */
void drawLabeledStatusCellsWorld00Area03004_801D7C10(u16 x, u16 y, u8 mask) {
  u8 flags[3];
  u8 shade;
  const char* label;
  s32 i;

  for (i = 0; i < 3; i++) {
    u8* cell;

    /* The original re-derives the cell address from the frame base every
     * iteration instead of carrying a pointer induction variable, so the
     * address is built in two steps here as well. */
    cell = flags;
    cell += i;
    *cell = (mask >> i) & 1;
    func_801D7D68((s16)(x + i * 0x30), (s16)y, *cell);
    shade = *cell;
    if (mask == 0) {
      shade++;
    }
    *cell = shade;
  }

  label = D_801E21C0;
  func_8014F800((s16)(x + 6), (s16)(y + 4), (u8)(7 - flags[0] * 7), 4,
                (u32)label);
  func_8014F800((s16)(x + 0x36), (s16)(y + 4), (u8)(7 - flags[1] * 7), 4,
                (u32)(label + 4));
  func_8014F800((s16)(x + 0x66), (s16)(y + 4), (u8)(7 - flags[2] * 7), 4,
                (u32)(label + 8));
}
