#include "bof3/bof3.h"

/* Pointer cell of the work record this overlay publishes and every record
 * handler of the target reads. This body takes the record's trail point from
 * the words at +0x18 and +0x1C and its two projected points from the record
 * addresses +0x34 and +0x18, and reads work byte 6 to select the shade triple
 * of the line.
 * @source 0x1F800044 @kind bss
 */
extern u8* D_1F800044;

/* Shared primitive cursor of the main executable: the draw-mode packet, the
 * flat line and both gouraud emitters of this body are seeded in it.
 * @source 0x8014598C @kind unknown
 */
extern u8* g_PrimCursor;

/* Shade triple of the overlay drawing tables at 0x801F6138: three consecutive
 * bytes per shade row, selected by work byte 6 times three, of which 0x801F6138
 * is the red member, 0x801F6139 the green and 0x801F613A the blue. This body
 * shades the flat line it links between the two record points.
 * @source 0x801F6138 @kind unknown
 */
extern u8 D_801F6138[];
/* @source 0x801F6139 @kind unknown */
extern u8 D_801F6139[];
/* @source 0x801F613A @kind unknown */
extern u8 D_801F613A[];

/* Shared effect bank byte the release arms zero or arm with 0x1F. A zero value
 * suppresses this body entirely.
 * @source 0x8014832E @kind data
 */
extern u8 D_8014832E;
/* Shared main-RAM flag bank the overlay reaches through func_8015B5D4; this
 * body draws only while flag 0x6A of the bank is set.
 * @source 0x80144F28 @kind data
 */
extern u8 D_80144F28;
/* Shared per-frame counter whose low bit the two extent conversions of this
 * body add to the converted amplitude.
 * @source 0x80143E6C @kind data
 */
extern u32 D_80143E6C;

/* Shared services of the main executable, outside this payload. */
u32 func_8015B5D4(void* flag_bank, u8 bit_index);
void func_80155A08(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
void func_801AFE18();
void func_8017AA80(void* primitive);
void func_801AFF04(const void* arg0, void* arg1);
void func_801AFFD8(const void* arg0, void* arg1, void* arg2);
s32 func_8017A138(s32 arg0, s32 arg1);

/* Gouraud emitters of this payload, both reached only from this body: the
 * eight-triangle fan samples the screen pair it is handed as its centre, the
 * two-quad emitter the two screen pairs it is handed as the near end of each
 * quad. */
void drawTranslucentEightTriangleGouraudFanAroundScreenPairTintedByWorkByte6TripleWorld03Area14813_801F4DF0(
    s32 arg0, s32 arg1, s32 arg2, s32 arg3);
void drawTranslucentTwoGouraudQuadsAroundScreenPairTintedByWorkByte6TripleWorld03Area14813_801F4A50(
    s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5, s32 arg6,
    u16 arg7);

/* One 8-byte projected screen entry: func_801AFFD8 writes the size pair the
 * caller seeds at +0x00/+0x02 and the converted extent halfword at +0x00 of its
 * output entry. The entries this body does not seed stay unwritten. */
typedef struct World03Area14813ScreenPoint {
  s16 x;
  s16 y;
  s16 unk_04;
  s16 unk_06;
} World03Area14813ScreenPoint;

/* @source 0x801F4798
 * @behavior Effect draw of this overlay. It runs only while the shared effect
 *           byte 0x8014832E is set and flag 0x6A of the main-RAM flag bank
 *           0x80144F28 is set, and it draws the beam between the work record's
 *           trail point (the record addresses +0x34 and +0x18 handed by both
 *           state-byte-1 handlers) and its current point. It queues a 0xC-byte
 *           draw-mode packet for tpage 0x3C0 with dtd 1, then projects the two
 *           record points into the two vertices of a flat line seeded in the
 *           shared primitive cursor through func_8017AA80, marks the line fully
 *           opaque and shades it with the table triple at 0x801F6138 + 3 *
 *           work byte 6. The heading between the two projected vertices comes
 *           from func_8017A138, the extent of both points at a 0x40-wide,
 *           0x00-high size pair from func_801AFFD8, so the drawn amplitude is
 *           the converted extent plus the low bit of 0x80143E6C. The eight
 *           triangle gouraud fan is drawn at the current point with the second
 *           extent and the heading one 0xC00 turn further on, and the two
 *           gouraud quads span the trail point and the current point with the
 *           0x400 and 0xC00 turned headings. The body then restores tpage 0x3C0
 *           with dtd 0 and queues a second 0xC-byte draw-mode packet. The
 *           record fixed-point position pair at +0x18/+0x1C is linked with the
 *           line and with the draw-mode packets through func_80155A08. Takes
 *           the two record addresses the callers pass as +0x34 and +0x18,
 *           returns nothing and keeps a 0x78-byte frame whose locals are the
 *           four-entry screen array and the 0x20-byte projection scratch region
 *           at +0x40 and whose saved registers are $s0-$s4 and ra.
 *
 * Name evidence: the body's own calls fix the name - one flat line between the
 *           two projected record points (record +0x34 first, +0x18 second), one
 *           gouraud fan at the second point and two gouraud quads spanning both
 *           points, all in the payload's own emitters.
 *
 * Spelling evidence: the size pair and the converted extent are the first two
 *           entries of the same four-entry 8-byte screen array, which is what
 *           puts the seeded pair at +0x20/+0x22 and the extent at +0x28 and
 *           what makes the extent load an unsigned halfword (the sum is stored
 *           as a signed halfword amplitude). The heading is converted to a
 *           halfword at its own use, which is what keeps the +0x400 offset in
 *           the argument list and the +0xC00 offset in its own local.
 *
 * Shape evidence: func_801AFE18 is called without an argument here - the
 *           delay slot at 0x801F482C is a nop - while the 0x20-byte scratch
 *           region is still reserved in the frame at +0x40, exactly where the
 *           exact sibling drawProjectedBeamWithRotatedEndQuadsAndGouraudBody-
 *           World03Area14514_801F332C hands its own scratch to the same callee.
 *           That sibling is shape evidence for the screen entry, the size-extent
 *           conversion and the projected line only: this payload draws through
 *           its own fan and quad emitters and keeps no projected array, so the
 *           span is not a clone candidate.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawLineGouraudFanAndTwoGouraudQuadsBetweenRecordPoints34And18World03Area14813_801F4798(
    s32 arg0, s32 arg1) {
  World03Area14813ScreenPoint screen[4];
  u8 scratch[0x20];
  LINE_F2* prim;
  s16 amp0;
  s16 amp1;
  s32 angle;
  u16 ang0;
  u16 ang1;

  if (D_8014832E != 0) {
    if (func_8015B5D4(&D_80144F28, 0x6A) != 0) {
      SetDrawMode((DR_MODE*)g_PrimCursor, 0, 1, GetTPage(0, 1, 0x3C0, 0), 0);
      func_80155A08(FIELD_REF(s32, D_1F800044, 0x18u),
                    FIELD_REF(s32, D_1F800044, 0x1Cu), 0, 0xC);
      func_801AFE18();
      prim = (LINE_F2*)g_PrimCursor;
      func_8017AA80(prim);
      SetSemiTrans(prim, 0);
      func_801AFF04((const void*)arg0, &prim->x0);
      func_801AFF04((const void*)arg1, &prim->x1);
      prim->r0 = D_801F6138[D_1F800044[6] * 3];
      prim->g0 = D_801F6139[D_1F800044[6] * 3];
      prim->b0 = D_801F613A[D_1F800044[6] * 3];
      func_80155A08(FIELD_REF(s32, D_1F800044, 0x18u),
                    FIELD_REF(s32, D_1F800044, 0x1Cu), 0, 0x10);
      angle = func_8017A138(prim->y1 - prim->y0, prim->x1 - prim->x0);
      screen[0].x = 0x40;
      screen[0].y = 0;
      func_801AFFD8((const void*)arg0, &screen[0], &screen[1]);
      amp0 = screen[1].x + (D_80143E6C & 1);
      func_801AFFD8((const void*)arg1, &screen[0], &screen[1]);
      amp1 = screen[1].x + (D_80143E6C & 1);
      ang1 = angle + 0xC00;
      drawTranslucentEightTriangleGouraudFanAroundScreenPairTintedByWorkByte6TripleWorld03Area14813_801F4DF0(
          prim->x1, prim->y1, amp1, ang1);
      drawTranslucentTwoGouraudQuadsAroundScreenPairTintedByWorkByte6TripleWorld03Area14813_801F4A50(
          prim->x0, prim->y0, amp0, (u16)(angle + 0x400), prim->x1, prim->y1,
          amp1, ang1);
      SetDrawMode((DR_MODE*)g_PrimCursor, 0, 0, GetTPage(0, 1, 0x3C0, 0), 0);
      func_80155A08(FIELD_REF(s32, D_1F800044, 0x18u),
                    FIELD_REF(s32, D_1F800044, 0x1Cu), 0, 0xC);
    }
  }
}
