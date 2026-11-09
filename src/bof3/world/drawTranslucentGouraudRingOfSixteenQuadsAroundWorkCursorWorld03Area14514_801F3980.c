#include "bof3/bof3.h"
#include "gpu/prim.h"

/* Shared resident helpers, declared the way this overlay's exact siblings
 * declare them: func_801AFE18 fills the projection scratch handed as arg0,
 * the projector func_801AFF04 converts the three-word point handed as arg0 into
 * the screen halfword pair written at arg1, func_801782FC and func_801783C8
 * return the sine and cosine of a 12-bit phase in the scale of the tables they
 * index, and func_80155A08 links one grid-cell primitive at the passed packed
 * coordinate pair with its kind and size class. */
void func_801AFE18(void* arg0);
void func_801AFF04(const void* arg0, void* arg1);
s32 func_801782FC(s32 arg0);
s32 func_801783C8(s32 arg0);
void func_80155A08(s32 arg0, s32 arg1, s32 arg2, s32 arg3);

/* Shared per-frame flip flag, the same cell the exact sibling
 * drawProjectedBeamWithRotatedEndQuadsAndGouraudBodyWorld03Area14514_801F332C
 * biases its extents with. */
extern s32 D_80143E6C; /* @source 0x80143E6C @kind data */

/* One 8-byte projected screen entry: func_801AFF04 writes the halfword pair at
 * +0x00/+0x02 and the trailing four bytes stay unwritten, which is why the
 * band's own pair copy only moves the leading four bytes. The pair is a type of
 * its own because the original copies it as one four-byte unaligned block. */
typedef struct World03Area14514ScreenPair {
  u16 x;
  u16 y;
} World03Area14514ScreenPair;

typedef struct World03Area14514ScreenPoint {
  u16 x;
  u16 y;
  u16 unk_04;
  u16 unk_06;
} World03Area14514ScreenPoint;

/* @source 0x801F3980
 * @behavior Draws the overlay's cursor band: a closed translucent gouraud ring
 *   of sixteen quads around the three-word work-cursor position handed in
 *   cursor.
 *   It initialises the frame's projection scratch through func_801AFE18, then
 *   seeds the last two screen entries from the position offset in x and y by the
 *   cosine and sine of phase zero, scaled by 0x200 and shifted right four, and
 *   projects that point twice: at the position's own depth word for the near
 *   entry and at that depth plus 0x08000000 for the far entry.
 *   Each of the sixteen steps advances the phase by 0x100, copies the previous
 *   pair of projected entries into the first two entries of the four-entry
 *   array, rebuilds the point at the cosine and sine of the masked phase and
 *   projects both depths again. The step then publishes the draw mode for
 *   texture page (0, 1, 0x3C0, 0) with dither on, links the 0x0C-byte
 *   draw-mode cell through func_80155A08 with kind 0, and fills one
 *   semi-transparent POLY_G4 whose first two vertices are the previous pair and
 *   whose last two are the freshly projected pair, so successive quads stitch
 *   into the ring. The four vertices share one grey level that starts at 0x20
 *   biased by four times the low bit of D_80143E6C; the far pair takes the level
 *   after the step that the quad index selects, which lowers it by four while
 *   the index is masked to 4..0x0B and raises it by four outside that window, so
 *   the level climbs across the first four quads, falls across the next eight
 *   and climbs again across the last four. The quad is linked through
 *   func_80155A08 with the point's own x and y, kind 0 and size 0x24, then the
 *   draw mode is published once more with dither off and its 0x0C-byte cell
 *   linked for the next step. The loop ends when the masked index reaches
 *   sixteen, and nothing is returned.
 * Frame evidence: the 0xC0-byte frame holds the four-entry screen array at
 *   0x18, the sixteen-byte VECTOR point at 0x38 that both projections read back,
 *   and the 0x50-byte projection scratch at 0x48, whose address is the only one
 *   of the body's local addresses that escapes; the save block at 0x98 fixes the
 *   local span and therefore the scratch size.
 * Name evidence: the caller drawWorkCursorQuadWorld03Area14514_801F3938 copies
 *   the work record's cursor triple at +0x34/+0x38/+0x3C into the point it hands
 *   over and documents this address as the overlay's quad renderer, and the body
 *   emits sixteen translucent gouraud quads around that point, so the name
 *   states the ring and its cursor instead of an unproven effect role.
 * Shape evidence: the projection scratch, the three-word point handed to
 *   func_801AFF04, the 8-byte projected entries and the phase-stepped quad walk
 *   are this repository's lifted family, e.g.
 *   drawTexturedRibbonAlongWorkSpanWorld03Area13213_801F3268 and
 *   drawProjectedBeamWithRotatedEndQuadsAndGouraudBodyWorld03Area14514_801F332C;
 *   those bodies carry their own textures, spans and record layouts, so they are
 *   shape evidence only and every offset, constant, count and helper binding
 *   here comes from this function's own original bytes.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawTranslucentGouraudRingOfSixteenQuadsAroundWorkCursorWorld03Area14514_801F3980(
    s32* cursor) {
  World03Area14514ScreenPoint screen[4];
  VECTOR point;
  u8 scratch[0x50];
  POLY_G4* prim;
  s32 phase;
  u16 angle;
  s32 color;
  s32 i;

  func_801AFE18(scratch);
  phase = 0;
  point.vx = cursor[0] + ((func_801783C8(phase) * 0x200) >> 4);
  point.vy = cursor[1] + ((func_801782FC(phase) * 0x200) >> 4);
  point.vz = cursor[2];
  func_801AFF04(&point, &screen[2]);
  point.vz = cursor[2] + 0x08000000;
  func_801AFF04(&point, &screen[3]);
  color = 0x20 | ((D_80143E6C & 1) << 2);
  i = 0;

  do {
    *(World03Area14514ScreenPair*)&screen[0] =
        *(World03Area14514ScreenPair*)&screen[2];
    *(World03Area14514ScreenPair*)&screen[1] =
        *(World03Area14514ScreenPair*)&screen[3];
    phase += 0x100;
    angle = phase;
    point.vx = cursor[0] + ((func_801783C8(angle) * 0x200) >> 4);
    point.vy = cursor[1] + ((func_801782FC(angle) * 0x200) >> 4);
    point.vz = cursor[2];
    func_801AFF04(&point, &screen[2]);
    point.vz = cursor[2] + 0x08000000;
    func_801AFF04(&point, &screen[3]);

    SetDrawMode((DR_MODE*)g_PrimCursor, 0, 1, GetTPage(0, 1, 0x3C0, 0), 0);
    func_80155A08(point.vx, point.vy, 0, 0xC);

    prim = (POLY_G4*)g_PrimCursor;
    SetPolyG4(prim);
    SetSemiTrans(prim, 1);
    prim->x0 = screen[0].x;
    prim->y0 = screen[0].y;
    prim->x1 = screen[1].x;
    prim->y1 = screen[1].y;
    prim->x2 = screen[2].x;
    prim->y2 = screen[2].y;
    prim->x3 = screen[3].x;
    prim->y3 = screen[3].y;
    prim->r0 = color;
    prim->g0 = color;
    prim->b0 = color;
    prim->r1 = color;
    prim->g1 = color;
    prim->b1 = color;
    if ((u8)(i - 4) >= 8) {
      color += 4;
    } else {
      color -= 4;
    }
    prim->r2 = color;
    prim->g2 = color;
    prim->b2 = color;
    prim->r3 = color;
    prim->g3 = color;
    prim->b3 = color;
    func_80155A08(point.vx, point.vy, 0, 0x24);

    SetDrawMode((DR_MODE*)g_PrimCursor, 0, 0, GetTPage(0, 1, 0x3C0, 0), 0);
    func_80155A08(point.vx, point.vy, 0, 0xC);
    i++;
  } while ((u8)i < 0x10);
}
