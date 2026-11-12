#include "bof3/bof3.h"

/* One entry of the per-kind table at 0x801D4DD4, in kind order: the low byte at
 * 0x801D4DD4 is the index of the first name run of that kind inside the name
 * run blob and the high byte at 0x801D4DD5 is how many name runs, and so how
 * many entry rows, the kind has. Kinds 0 through 3 carry rows, the rest carry
 * none. */
typedef struct Commu0107KindRows {
  u8 first; /* +0x00 */
  u8 count; /* +0x01 */
} Commu0107KindRows;

extern Commu0107KindRows D_801D4DD4[]; /* @source 0x801D4DD4 @kind data */

/* Shared clut byte that both backing rectangles of the panel take. */
extern u8 D_80144952; /* @source 0x80144952 @kind data */

/* Eight-cell commu entry names of this overlay, one run per entry row and NUL
 * padded, twenty runs in all: the kind grid starts at the eleventh, the run at
 * 0x801D4D84. */
extern u8 commuEntryNameStrings[]; /* @source 0x801D4D34 @kind data */

extern void func_801AE3F0(u16 x, u16 y, u32 width, u32 height, u32 shade,
                          u32 clut); /* @source 0x801AE3F0 */
extern void func_8014F800(s16 x, s16 y, u8 align, u32 arg3,
                          u32 arg4); /* @source 0x8014F800 */
extern void drawEntryListFrame(s16 x, s16 y, u8 height);

/* @source 0x801D44E0
 * @behavior Draws the commu01/07 kind-grid panel, the entry-list panel of one
 * commu kind. The per-kind table at 0x801D4DD4 gives each kind the index of its
 * first name run and its row count, so the panel height is that row count times
 * sixteen and the panel fills that many entry rows. While the origin plus that
 * height plus the panel's four-pixel bottom pad reaches the 0xD9 screen bound
 * the origin moves up to 0xD4 minus the height. While the mode byte is zero it
 * first paints the inset rectangle through func_801AE3F0 at (x + 4, y + 6), 0x30
 * wide and that height tall, with shade 0x81; it then paints the backing
 * rectangle at (x, y) 0x50 wide and the same height with shade 0x80. Both take
 * the shared clut byte 0x80144952. It draws the entry-list frame through
 * drawEntryListFrame at (x, y - 1) with the height rounded down to a multiple of
 * sixteen and, while the kind has rows, submits one caption per row through
 * func_8014F800 at (x + 8, y + 2 plus sixteen per row) with alignment seven
 * times the mode byte and cell height 8, reading the eight-cell name run at
 * 0x801D4D84 advanced by that kind's first run index and the row index. It
 * returns the origin it settled on. Its in-image callers func_801D1A94,
 * func_801D1E48 and func_801D2110 pass an x built from the quarter-step origin
 * 0x4A or 0x52, their own y, the overlay cursor kind bytes 0x801D4E1C or
 * 0x801D4E20 and mode 0 or 1. No other state is read or written.
 * @status exact
 * @match 100.00
 * @residual none
 */
s16 drawKindGridPanel(s16 x, s16 y, u8 kind, u8 mode) {
  s32 bottom;
  s32 i;
  s32 height;
  u16 span;

  height = D_801D4DD4[kind].count << 4;
  span = height;
  bottom = span + 4;
  if (y + bottom >= 0xD9) {
    y = 0xD4 - height;
  }
  if (mode == 0) {
    func_801AE3F0((u16)(x + 4), (u16)(y + 6), 0x30, span, 0x81, D_80144952);
  }
  func_801AE3F0((u16)x, (u16)y, 0x50, height | 1, 0x80, D_80144952);
  drawEntryListFrame((s16)x, (s16)(y - 1), span & 0xF0);
  i = 0;
  if (i < D_801D4DD4[kind].count) {
    do {
      func_8014F800((s16)(x + 8), (s16)(y + 2 + i * 16), mode * 7, 8,
                    (u32)&commuEntryNameStrings[0x50 +
                                                (D_801D4DD4[kind].first + i) * 8]);
      i++;
    } while (i < D_801D4DD4[kind].count);
  }
  return y;
}
