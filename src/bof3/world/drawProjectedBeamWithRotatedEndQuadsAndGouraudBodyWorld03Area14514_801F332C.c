#include "bof3/bof3.h"
#include "gpu/prim.h"

/* Shared resident helpers, declared the way this overlay's exact siblings
 * declare them: func_801AFE18 fills the projection scratch, the projector
 * func_801AFF04 converts the three-word point handed as arg0 into the screen
 * halfword pair at arg1, func_801AFFD8 converts the size pair at arg1 through
 * the same transform and writes the result at arg2, and func_8017A138 returns
 * the 0x1000-unit angle of the signed delta (arg0, arg1). */
void func_801AFE18(void* arg0);
void func_801AFF04(const void* arg0, void* arg1);
void func_801AFFD8(const void* arg0, void* arg1, void* arg2);
s32 func_8017A138(s32 arg0, s32 arg1);

/* This overlay's own two drawing helpers: 0x801F34C8 emits the translucent
 * quad rotated a quarter turn per vertex around the passed screen point at the
 * passed extent and phase, and 0x801F3618 emits the semi-transparent gouraud
 * quad spanned by the two screen pairs with one converted extent and phase at
 * each end. */
void func_801F34C8(s16 arg0, s16 arg1, s16 arg2, u16 arg3);
void func_801F3618(s16 arg0, s16 arg1, s16 arg2, u16 arg3, s16 arg4, s16 arg5,
                   s16 arg6, u16 arg7);

extern s32 D_80143E6C; /* @source 0x80143E6C @kind data */

/* One 8-byte projected screen entry: func_801AFF04 writes the halfword pair at
 * +0x00/+0x02 and func_801AFFD8 writes the converted size halfword at +0x00 of
 * its output entry. The trailing four bytes of each entry stay unwritten. */
typedef struct World03Area14514ScreenPoint {
  s16 x;
  s16 y;
  s16 unk_04;
  s16 unk_06;
} World03Area14514ScreenPoint;

/* @source 0x801F332C
 * @behavior Overlay drawing helper for the work record's two position vectors.
 * It sets the drawing state for texture page (0, 1, 0x3C0, 0) through
 * GetTPage and SetDrawMode on the current primitive cursor, appends the 0x0C
 * draw-mode packet with slot 1, and fills the 0x20-byte projection scratch of
 * func_801AFE18. Both passed three-word points are then projected into their
 * own eight-byte screen entries, and the direction of the projected segment
 * sets the beam heading through func_8017A138 over the screen deltas.
 * The size pair {0x10, 0} is converted once per end point: func_801AFFD8
 * writes the converted width into the third entry, and each end point's extent
 * is that width biased by the low bit of D_80143E6C, which is why the two ends
 * share the pair but not the result. The heading is offset by 0x400 for the
 * first end and by 0xC00 for the second, each masked to 16 bits, so the two
 * rotated end quads of func_801F34C8 are a quarter turn apart. Finally
 * func_801F3618 receives both projected screen pairs with their own extents
 * and phases and emits the gouraud body quad between them. Nothing is returned
 * and no other state is read.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawProjectedBeamWithRotatedEndQuadsAndGouraudBodyWorld03Area14514_801F332C(
    const void* arg0, const void* arg1) {
  World03Area14514ScreenPoint screen[4];
  u8 scratch[0x20];
  u16 tpage;
  s16 first_extent;
  s16 second_extent;
  s32 heading;
  u16 first_angle;
  u16 second_angle;

  tpage = GetTPage(0, 1, 0x3C0, 0);
  SetDrawMode((DR_MODE*)g_PrimCursor, 0, 1, tpage, 0);
  GpuAppendPrim(1, 0xC);
  func_801AFE18(scratch);

  func_801AFF04(arg0, &screen[0]);
  func_801AFF04(arg1, &screen[1]);
  heading = func_8017A138(screen[1].y - screen[0].y, screen[1].x - screen[0].x);

  screen[2].x = 0x10;
  screen[2].y = 0;
  func_801AFFD8(arg0, &screen[2], &screen[3]);
  first_angle = (u16)(heading + 0x400);
  first_extent = screen[3].x + (D_80143E6C & 1);
  func_801F34C8(screen[0].x, screen[0].y, first_extent, first_angle);

  func_801AFFD8(arg1, &screen[2], &screen[3]);
  second_angle = (u16)(heading + 0xC00);
  second_extent = screen[3].x + (D_80143E6C & 1);
  func_801F34C8(screen[1].x, screen[1].y, second_extent, second_angle);

  func_801F3618(screen[0].x, screen[0].y, first_extent, first_angle,
                screen[1].x, screen[1].y, second_extent, second_angle);
}
