#include "bof3/bof3.h"
#include "gpu/prim.h"

/* One 8-byte screen-projected point of the address-taken local workspace this
 * drawer keeps on its frame: the shared projector func_801AFF04 writes the
 * projected x/y halfword pair at +0x00/+0x02 of an entry, and the shared extent
 * converter func_801AFFD8 writes the scaled signed pair into a second entry the
 * same way. Only the x/y halfword pair is proven here; the 8-byte stride comes
 * from the three entries the frame holds at 0x10, 0x18 and 0x20, and the
 * trailing four bytes of an entry stay unknown. */
typedef struct World01Area047ScreenPoint {
  u16 x;
  u16 y;
  u16 unk_04;
  u16 unk_06;
} World01Area047ScreenPoint;

/* Shared resident helpers. func_801AFF04 projects the three-word world
 * descriptor handed as arg0 into the screen pair at arg1, func_801AFFD8
 * converts the extent pair at arg1 into the scaled signed pair at arg2, and
 * func_801782FC/func_801783C8 return the sine and the cosine of a 12-bit phase
 * in the same fixed-point scale as the >> 12 products below. The prototypes
 * follow the exact siblings that call these helpers from the same resident
 * addresses. */
void func_801AFF04(const void* arg0, void* arg1);
void func_801AFFD8(const void* arg0, void* arg1, void* arg2);
s32 func_801782FC(s32 arg0);
s32 func_801783C8(s32 arg0);

/* @source 0x801F2F60
 * @behavior Per-frame effect step for one 0x18-byte work record of this
 *           overlay, called by
 *           updateWorkTableRecordsAndReportActiveWorld01Area04713_801F330C
 *           with the record pointer once its sub-phase byte +0x15 reaches 1.
 *           It projects the record's three-word world descriptor at +0x00 into
 *           the first entry of its local screen-point workspace through the
 *           shared projector func_801AFF04, then makes two passes over the
 *           whole 12-bit turn. Each pass builds the extent input pair from the
 *           record: the first pass uses the signed phase halfword +0x10 plus
 *           the amplitude halfword +0x12, the second pass uses their
 *           difference, and both fill the second halfword of that pair with the
 *           unsigned phase again before handing it to func_801AFFD8, which
 *           returns the signed radial extent pair. Inside a pass it emits 32
 *           translucent Gouraud quads, one per 0x80 of the turn: each quad
 *           takes its two leading vertices from the first returned extent at
 *           the current phase and at the phase plus 0x80, its two trailing
 *           vertices from the second extent at the same two angles, and every
 *           vertex offsets the projected base pair by the sine and cosine
 *           product of that extent shifted right 12. The two leading vertices
 *           are written black and the two trailing ones carry the record's
 *           countdown byte +0x17 in all three channels, so the band fades from
 *           the tinted outer edge to black at the inner one while that byte
 *           counts the effect down. The phase advances by 0x80 per quad and the
 *           pass ends when it reaches 0x1000, and each quad is linked through
 *           the shared primitive appender as a 0x24-byte entry at ordering
 *           table index 1. Takes the record pointer in $a0, returns nothing,
 *           and keeps a 0x48-byte frame for $ra, $s5, $s4, $s3, $s2, $s1 and
 *           $s0 over the 0x10..0x27 span of its three-entry workspace.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawFadingTranslucentGouraudRingPairWorld01Area04713_801F2F60(u8* record) {
  World01Area047ScreenPoint point[3];
  POLY_G4* prim;
  u16 phase;
  u16 next;
  u16 i;

  func_801AFF04(record, &point[0]);
  i = 0;
  do {
    point[1].x = FIELD_REF(s16, record, 0x10) +
        (i == 0 ? FIELD_REF(s16, record, 0x12)
                : -FIELD_REF(s16, record, 0x12));
    point[1].y = FIELD_REF(u16, record, 0x10);
    func_801AFFD8(record, &point[1], &point[2]);
    phase = 0;
    do {
      prim = (POLY_G4*)g_PrimCursor;
      SetPolyG4(prim);
      SetSemiTrans(prim, 1);
      next = (phase + 0x80) & 0xFFF;
      prim->x0 = point[0].x + (((s16)point[2].x * func_801783C8(phase)) >> 12);
      prim->y0 = point[0].y + (((s16)point[2].x * func_801782FC(phase)) >> 12);
      prim->x1 = point[0].x + (((s16)point[2].x * func_801783C8(next)) >> 12);
      prim->y1 = point[0].y + (((s16)point[2].x * func_801782FC(next)) >> 12);
      prim->x2 = point[0].x + (((s16)point[2].y * func_801783C8(phase)) >> 12);
      prim->y2 = point[0].y + (((s16)point[2].y * func_801782FC(phase)) >> 12);
      prim->x3 = point[0].x + (((s16)point[2].y * func_801783C8(next)) >> 12);
      prim->y3 = point[0].y + (((s16)point[2].y * func_801782FC(next)) >> 12);
      prim->r0 = 0;
      prim->g0 = 0;
      prim->b0 = 0;
      prim->r1 = 0;
      prim->g1 = 0;
      prim->b1 = 0;
      prim->r2 = FIELD_REF(u8, record, 0x17);
      prim->g2 = FIELD_REF(u8, record, 0x17);
      prim->b2 = FIELD_REF(u8, record, 0x17);
      prim->r3 = FIELD_REF(u8, record, 0x17);
      prim->g3 = FIELD_REF(u8, record, 0x17);
      prim->b3 = FIELD_REF(u8, record, 0x17);
      phase += 0x80;
      GpuAppendPrim(1, 0x24);
    } while (phase < 0x1000);
    i++;
  } while (i < 2);
}
