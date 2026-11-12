#include "bof3/bof3.h"
#include "gpu/prim.h"

/* Shaded-rectangle filler of the resident executable: paints the rectangle at
 * (x, y) with the given size, shade byte and clut byte. */
extern void func_801AE3F0(u16 x, u16 y, u32 width, u32 height, u32 shade,
                          u32 clut); /* @source 0x801AE3F0 */

/* Inset-rectangle filler of the resident executable: paints the rectangle at
 * (x, y) with the given size and shade byte. */
extern void func_801AED54(s32 x, s32 y, s32 width, s32 height,
                          s32 shade); /* @source 0x801AED54 */

/* Text emitter of the resident executable: draws the string it is handed at
 * (x, y). */
extern void func_8014FF0C(s16 x, s16 y, s32 arg2,
                          const void* arg3); /* @source 0x8014FF0C */

/* Graphics-type query of the running executable, owned by the SDK map. */
extern s32 GetGraphType(void);

/* Shared clut byte of the panels the resident executable paints. */
extern u8 D_80144952; /* @source 0x80144952 @kind data */

/* Shared frame counter of the main executable; its low bits select the blink
 * phase of the message-row highlight. */
extern s32 D_80143E6C; /* @source 0x80143E6C @kind unknown */

/* Pending world id and the commu entry count of that world, one byte of the
 * table at 0x801F286D indexed by it. */
extern u16 D_80143F00; /* @source 0x80143F00 @kind data */
extern u8 D_801F286D[]; /* @source 0x801F286D @kind data */

/* Result halfword of the current commu request, read as a text id: 0xFFFF
 * draws no message, 0x3D additionally draws the highlight tile of the message
 * row. */
extern u16 D_801D4E0C; /* @source 0x801D4E0C @kind data */

/* Toggle byte of the message row: cleared when the request result becomes
 * 0x3D, it selects the column the highlight tile covers. */
extern u8 D_801D4E18; /* @source 0x801D4E18 @kind data */

/* Signed commu counter printed in the first caption field. */
extern s8 D_801455C2; /* @source 0x801455C2 @kind unknown */

/* Village-expansion counter printed in the second caption field. */
extern u8 D_801455C4; /* @source 0x801455C4 @kind unknown */

/* Shared text buffer the two caption fields print into before submission. */
extern u8 D_80145AD4[]; /* @source 0x80145AD4 @kind unknown */

/* Halfword offset run on the 0x80010000 main-RAM base: the offset the request
 * result halfword selects from it is added to that base to form the message
 * handle func_8014F800 takes as its fifth argument. */
/* Kind byte and variant byte of each of the eight stride-8 local record
 * slots. */
extern u8 D_801457A8[][8]; /* @source 0x801457A8 @kind data */

/* Two-digit field format "%2d" of this overlay's data run. */
extern const char D_801D0C10[]; /* @source 0x801D0C10 @kind data */

/* Plain-decimal field format "%d" of this overlay's data run. */
extern const char D_801D0C14[]; /* @source 0x801D0C14 @kind data */

/* Caption of the second caption field as this overlay's data run holds it. */
extern const char D_801D4C30[]; /* @source 0x801D4C30 @kind data */

/* Byte run the record kind selects an icon ordinal from, except kind 5 which
 * takes the byte at 0x801D4C39 plus the variant byte of its own slot. */
extern u8 D_801D4C34[]; /* @source 0x801D4C34 @kind data */
extern u8 D_801D4C39; /* @source 0x801D4C39 @kind data */

extern void drawWindowTilePiece(s16 x, s16 y, u8 piece);
extern void drawWindowFrameCommu0107_801D3594(s16 x, s16 y);
extern void drawCommuEntryKindIconSprite(s16 x, s16 y, u8 icon);
extern u8 countActiveRecordsByKind(s32 kind);
extern void computeKindSlotOrigin(s16* outX, s16* outY, s32 kind, s32 ordinal);

/* @source 0x801D3038
 * @behavior Draws the commu01/07 commu entry-list screen: the message row, the
 *           list panel and window frame, the eight record-kind cells with their
 *           kind icons and record slots, and the two captions. It first paints
 *           the message-row backings through func_801AE3F0 at (0x14, 0x12) 0x118
 *           by 0x13 with shade 0xF2 and func_801AED54 at (0x16, 0x14) 0x113 by
 *           0xF, both taking the shared clut byte 0x80144952. While the request
 *           result halfword 0x801D4E0C is not 0xFFFF it submits that result as a
 *           text id through func_8014F800 at (0x1D, 0x15) with mask 0xFF,
 *           carrying the 0x80010000 main-RAM base plus the halfword offset the
 *           id selects from the table on that base, and while the result is the
 *           0x3D the step-5 confirm publishes it additionally stamps a
 *           semi-transparent TILE at the shared cursor: a second graphics-type
 *           query follows the first when that query does not answer 1, the
 *           draw-mode packet carries no texture page, flags or window, and the
 *           tile spans x 0xF4 plus 32 per unit of the toggle byte 0x801D4E18
 *           with width 0x19 less 8 per unit of it at y 0x16 and height 0xC,
 *           tinted from bits 1 and 2 of the shared frame counter 0x80143E6C
 *           scaled by 32 and offset by 0x3F, inverted while bit 3 of that
 *           counter is clear, so the highlight blinks. It then paints the list
 *           backing through func_801AE3F0 at (0x15, 0x2E) 0x114 by 0xAA with
 *           shade 0x80, draws the window frame through
 *           drawWindowFrameCommu0107_801D3594 at (0x14, 0x2D) and the header
 *           strip through func_801AED54 at (0x20, 0x38) 0xA8 by 0x10, and draws
 *           one tile piece per active record of kind 0 at x 0x24 plus 8 per
 *           record on y 0x3A. Three band rows follow at x 0xF2 and y 0x36 plus
 *           54 per row, each 0x30 square with shade 1: the row draws the tile
 *           piece at (0xEF, 0x33 plus that band) numbered 3 plus the row, then
 *           one 8-pixel cell per active record of kind row + 9 at x 0xF6 plus 8
 *           per column of five and at the band plus 7 per column band plus
 *           0x44. It then walks the eight record-kind cells at columns 0x20 plus
 *           52 per column and bands 0x6B plus 61 per band, each through
 *           func_801AED54 with size 0x28 and shade 1: a cell the pending world's
 *           entry count (0x801F286D indexed by 0x80143F00) does not reach is
 *           filled through func_801AE3F0 with shade 1 instead, while a cell it
 *           reaches draws tile piece 0 at (x + 0x1A, y + 0x5B), then, for a
 *           nonzero kind byte of its stride-8 slot, the kind icon through
 *           drawCommuEntryKindIconSprite at (x + 0x22, y + 0x6C) with the icon
 *           ordinal the kind selects from 0x801D4C34, or 0x02 plus the slot
 *           variant byte through the byte at 0x801D4C39 for kind 5, then one
 *           tile piece per ordinal of that kind at the origin
 *           computeKindSlotOrigin returns for kind i + 1 and ordinals 1 through
 *           the kind's active record count. Two caption fields close the body on
 *           the footer row of tile piece 2 at (0x30, 0x4C): the signed counter
 *           0x801455C2 printed with the "%2d" format at 0x801D0C10 into the
 *           shared text buffer 0x80145AD4 and submitted through func_8014FF0C at
 *           (0x50, 0x4C), the caption at 0x801D4C30 submitted through
 *           func_8014F800 at (0x78, 0x4C) with mask 7, and the counter
 *           0x801455C4 printed with the "%d" format at 0x801D0C14 into the same
 *           buffer and submitted at (0xB8, 0x4C). It takes no arguments, returns
 *           no value and writes no memory of its own beyond the primitive
 *           cursor.
 * @status partial
 * @match 83.67
 * @residual The body reproduces the frame size, both stack locals, every store,
 *           call argument and loop bound at the original 1372-byte span, but
 *           two instructions are placed differently and the loop counter of
 *           the three walk loops sits in s3 where the original holds it in s4,
 *           which rotates the promoted cell byte, both cell origins and the
 *           two drawn corners through both walks. The drawn-cell arm computes
 *           its two icon arguments once at the shared call where the original
 *           repeats them inside each arm, and the row-band offset is hoisted
 *           out of the cell loop where the original adds it inside. Seven
 *           clean-C shapes for the counter type and reuse, the cell-byte
 *           binding, the arm order, the origin spelling, the offset
 *           association, the blink spelling and the ordinary mark all kept the
 *           same counter register; the blink spelling without its own tint
 *           local measured 287 of 343 where the tint spelling measured 286, and
 *           the cell origins as un-offset bases measured 287 where the
 *           pre-offset corners measured 281.
 */
void drawCommuEntryListScreen(void) {
  TILE* tile;
  s32 band;
  s32 x;
  s32 y;
  s32 sel;
  u8 color;
  u8 i;
  u8 j;
  u8 n;
  u8 rem;
  u8 line;
  s16 origin[4];

  func_801AE3F0(0x14, 0x12, 0x118, 0x13, 0xF2, D_80144952);
  func_801AED54(0x16, 0x14, 0x113, 0xF, 0);
  if (D_801D4E0C != 0xFFFF) {
    func_8014F800(0x1D, 0x15, 0, 0xFF,
                  0x80010000u + PSX_REF(u16, 0x80010000u + D_801D4E0C * 2));
    if (D_801D4E0C == 0x3D) {
      if (GetGraphType() != 1) {
        GetGraphType();
      }
      SetDrawMode((DR_MODE*)g_PrimCursor, 0, 0, 0, 0);
      GpuAppendPrim(1, 0xC);
      if (D_80143E6C & 8) {
        color = ((D_80143E6C & 6) << 5) | 0x3F;
      } else {
        color = ((~D_80143E6C & 6) << 5) | 0x3F;
      }
      tile = (TILE*)g_PrimCursor;
      SetTile(tile);
      SetSemiTrans(tile, 1);
      sel = D_801D4E18;
      tile->y0 = 0x16;
      tile->h = 0xC;
      tile->r0 = color;
      tile->g0 = color;
      tile->b0 = 0;
      tile->x0 = (sel << 5) + 0xF4;
      tile->w = 0x19 - (sel << 3);
      GpuAppendPrim(1, 0x10);
    }
  }
  func_801AE3F0(0x15, 0x2E, 0x114, 0xAA, 0x80, D_80144952);
  drawWindowFrameCommu0107_801D3594(0x14, 0x2D);
  func_801AED54(0x20, 0x38, 0xA8, 0x10, 1);

  n = countActiveRecordsByKind(0);
  for (i = 0; i < n; i++) {
    drawWindowTilePiece((s16)(i * 8 + 0x24), 0x3A, 1);
  }

  for (i = 0; i < 3; i++) {
    band = i * 54;
    func_801AED54(0xF2, (s16)(band + 0x36), 0x30, 0x30, 1);
    drawWindowTilePiece(0xEF, (s16)(band + 0x33), i + 3);
    n = countActiveRecordsByKind((u8)(i + 9));
    for (j = 0; j < n; j++) {
      rem = j % 5;
      line = j / 5;
      drawWindowTilePiece((s16)(rem * 8 + 0xF6),
                          (s16)(band + (line * 7 + 0x44)), 1);
    }
  }

  for (i = 0; i < 8; i++) {
    x = (i & 3) * 52;
    y = (i >> 2) * 61;
    func_801AED54(x + 0x20, y + 0x6B, 0x28, 0x28, 1);
    if (D_801F286D[D_80143F00] <= i) {
      func_801AE3F0(x + 0x20, y + 0x6B, 0x28, 0x28, 1, D_80144952);
    } else {
      drawWindowTilePiece((s16)(x + 0x1A), (s16)(y + 0x5B), 0);
      sel = D_801457A8[i][0];
      if (sel != 0) {
        if (sel == 5) {
          color = D_801D4C39 + D_801457A8[i][1];
        } else {
          color = D_801D4C34[sel];
        }
        drawCommuEntryKindIconSprite((s16)(x + 0x22), (s16)(y + 0x6C), color);
      }
      n = countActiveRecordsByKind((u8)(i + 1));
      for (j = 1; j <= n; j++) {
        computeKindSlotOrigin(&origin[0], &origin[1], (u8)(i + 1), (u8)j);
        drawWindowTilePiece(origin[0], origin[1], 1);
      }
    }
  }

  drawWindowTilePiece(0x30, 0x4C, 2);
  sprintf((char*)D_80145AD4, (const char*)D_801D0C10, D_801455C2);
  func_8014FF0C(0x50, 0x4C, 0, D_80145AD4);
  func_8014F800(0x78, 0x4C, 0, 7, (u32)D_801D4C30);
  sprintf((char*)D_80145AD4, (const char*)D_801D0C14, D_801455C4);
  func_8014FF0C(0xB8, 0x4C, 0, D_80145AD4);
}
