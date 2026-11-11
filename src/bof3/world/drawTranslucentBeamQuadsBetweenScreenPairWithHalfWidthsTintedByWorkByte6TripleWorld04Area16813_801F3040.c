#include "bof3/bof3.h"

/* Pointer cell of the work record this overlay publishes and every record
 * handler of the target reads. This body reads work byte 6 to select the shade
 * triple of the two near vertices and the words at 0xC and 0x10 as the
 * fixed-point position pair handed to the primitive linker.
 * @source 0x1F800044 @kind bss
 */
extern u8* D_1F800044;

/* Shared primitive cursor of the main executable, the primitive the first of
 * the two quads is seeded in; the second quad is the same 0x24-byte object
 * copied one size class further on.
 * @source 0x8014598C @kind unknown
 */
extern u8* g_PrimCursor;

/* Shade triple of the overlay drawing tables at 0x801F4B2C: three consecutive
 * bytes per shade row, selected by work byte 6 times three, of which 0x801F4B2C
 * is the red member, 0x801F4B2D the green and 0x801F4B2E the blue. The three
 * members bind at their own addresses, which are the three loads the table base
 * reaches, and the four far vertex shades of a quad stay black.
 * @source 0x801F4B2C @kind unknown
 */
extern u8 D_801F4B2C[];
/* @source 0x801F4B2D @kind unknown */
extern u8 D_801F4B2D[];
/* @source 0x801F4B2E @kind unknown */
extern u8 D_801F4B2E[];

/* Shared primitive linker of the main executable at 0x80155A08: the record's
 * fixed-point position pair, the ordering kind and the primitive size class.
 */
void func_80155A08(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
/* Shared rotator pair at 0x801782FC and 0x801783C8. This body takes the
 * 0x801783C8 sample for the x displacement and the 0x801782FC sample for the y
 * displacement of every spoke of both quads. */
s32 func_801782FC(s32 arg0);
s32 func_801783C8(s32 arg0);

/* @source 0x801F3040
 * @behavior Overlay gouraud quad emitter of the payload. Its only in-image
 *           reference is the jal at 0x801F3018 inside 0x801F2E10, which passes
 *           the record halfwords at 0x8 and 0xA as the first screen pair, the
 *           projected amplitude as the first half width, the shared angle plus
 *           0x400 as the first angle, the record halfwords at 0xC and 0xE as
 *           the second screen pair, the same projected amplitude as the second
 *           half width and the shared angle plus 0xC00 as the second angle. The
 *           body seeds one POLY_G4 in the shared primitive cursor g_PrimCursor
 *           through SetPolyG4, marks it semi-transparent, writes the two screen
 *           pairs as its first two vertices and lays its two far vertices on the
 *           rotator samples 0x801783C8 and 0x801782FC of the two angles, each
 *           sample scaled by one signed half width through a shift of 12: the
 *           first far vertex leaves the first screen pair along the first angle
 *           with the first half width, the second leaves the second screen pair
 *           along the second angle plus a half turn with the second half width.
 *           The two near vertices take the shade triple at 0x801F4B2C + 3 * work
 *           byte 6 and the four far vertex shades are black. func_80155A08 then
 *           links the record fixed-point position pair at 0xC/0x10 with kind 1
 *           and the 0x24 size class. The 0x24-byte primitive is copied one size
 *           class further on and its two far vertices are rewritten as the same
 *           pair of spokes turned half a turn round - the first screen pair
 *           along the first angle plus 0x800 with the first half width, the
 *           second screen pair along the bare second angle with the second half
 *           width - and the copy is linked by a second func_80155A08 call with
 *           the same position pair, kind 1 and 0x24 size class.
 *
 * Name evidence: the scenario lift drawTranslucentBeamBodyQuadsBetweenTwoProjectedPointsWithHalfWidths
 *           is this body's exact shape in another overlay - same eight-parameter
 *           u16/s16 signature, same 0x24-byte POLY_G4 pair, same two rotator
 *           samples per far vertex over a 0x1000 turn with the 0x800 half turn
 *           separating the two quads - and fixes the roles: two screen points
 *           with one half width each. This body differs in its own immediates
 *           only, so its name takes that geometry and adds the two facts the
 *           bytes here show: the near-vertex shade triple at 0x801F4B2C read as
 *           3 * work byte 6, and the far vertex shades cleared.
 *
 * Spelling evidence: the six vertex inputs are u16 and the two half widths s16
 *           parameters rather than locals, which is what makes the original
 *           spill them as halfwords into the six slots at 0x10/0x18/0x20/0x28/
 *           0x30/0x38, reload the two angles with lhu and sign-extend a half
 *           width with a shift pair at each multiply instead of reloading a
 *           halfword slot. The shade row index is a halfword - three times a
 *           byte, at most 0x2FD - which is why the six tint loads shape the
 *           index as u16; the word spelling reserves three 8-byte stack
 *           temporaries in this body and lifts every saved register by 0x18.
 *
 * Shape evidence: not a clone. The scenario sibling's 0x284-byte span and this
 *           0x3A0-byte span differ in length, in the tint run and in the linker
 *           call, so that lift supplies the parameter widths, the declaration
 *           style and the statement order only.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawTranslucentBeamQuadsBetweenScreenPairWithHalfWidthsTintedByWorkByte6TripleWorld04Area16813_801F3040(
    u16 x0, u16 y0, s16 width0, u16 ang0, u16 x1, u16 y1, s16 width1,
    u16 ang1) {
  POLY_G4* prim;

  prim = (POLY_G4*)g_PrimCursor;
  SetPolyG4(prim);
  SetSemiTrans(prim, 1);
  prim->x0 = x0;
  prim->y0 = y0;
  prim->x1 = x1;
  prim->y1 = y1;
  prim->x2 = x0 + ((width0 * func_801783C8(ang0)) >> 12);
  prim->y2 = y0 + ((width0 * func_801782FC(ang0)) >> 12);
  prim->x3 = x1 + ((width1 * func_801783C8(ang1 + 0x800)) >> 12);
  prim->y3 = y1 + ((width1 * func_801782FC(ang1 + 0x800)) >> 12);
  prim->r0 = D_801F4B2C[(u16)(D_1F800044[6] * 3)];
  prim->g0 = D_801F4B2D[(u16)(D_1F800044[6] * 3)];
  prim->b0 = D_801F4B2E[(u16)(D_1F800044[6] * 3)];
  prim->r1 = D_801F4B2C[(u16)(D_1F800044[6] * 3)];
  prim->g1 = D_801F4B2D[(u16)(D_1F800044[6] * 3)];
  prim->b1 = D_801F4B2E[(u16)(D_1F800044[6] * 3)];
  prim->r2 = 0;
  prim->g2 = 0;
  prim->b2 = 0;
  prim->r3 = 0;
  prim->g3 = 0;
  prim->b3 = 0;
  func_80155A08(FIELD_REF(s32, D_1F800044, 0xCu),
                FIELD_REF(s32, D_1F800044, 0x10u), 1, 0x24);
  prim++;
  *prim = prim[-1];
  prim->x2 = x0 + ((width0 * func_801783C8(ang0 + 0x800)) >> 12);
  prim->y2 = y0 + ((width0 * func_801782FC(ang0 + 0x800)) >> 12);
  prim->x3 = x1 + ((width1 * func_801783C8(ang1)) >> 12);
  prim->y3 = y1 + ((width1 * func_801782FC(ang1)) >> 12);
  func_80155A08(FIELD_REF(s32, D_1F800044, 0xCu),
                FIELD_REF(s32, D_1F800044, 0x10u), 1, 0x24);
}
