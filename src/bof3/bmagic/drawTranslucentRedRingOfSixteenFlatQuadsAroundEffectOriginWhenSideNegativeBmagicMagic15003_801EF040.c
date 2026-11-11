#include "bof3/bof3.h"
#include "gpu/prim.h"

/* Packet setter of the shared library at 0x8017A9A4: it writes the four-word
 * header of the flat-quad packet this body then fills. It sits between the
 * flat-triangle setter 0x8017A954 and the flat-textured-quad setter 0x8017A9B8
 * in the library image and no symbol in the shared maps names it, so it keeps
 * its raw name here, the way this overlay's exact siblings bind the setters
 * they call. */
void func_8017A9A4(void *arg0); /* @source 0x8017A9A4 */

/* Shared phase samplers at 0x801783C8 and 0x801782FC: the cosine and sine of a
 * 12-bit phase in the scale of the tables they index, which is why the cosine
 * product below is shifted right four and the sine product left four, the same
 * pairing the overlay's burst draw 0x801EF4E0 applies to the same two words. */
s32 func_801783C8(s32 arg0); /* @source 0x801783C8 */

s32 func_801782FC(s32 arg0); /* @source 0x801782FC */

/* Shared projector of the three-word point at 0x801AFF04: it converts the
 * point handed as arg0 into the screen halfword pair written at arg1. */
void func_801AFF04(const void *arg0, void *arg1); /* @source 0x801AFF04 */

/* Shared two-point cross-product test at 0x801AFF90: it reads three two-halfword
 * points and returns the low halfword of (b - a) x (c - b) already
 * sign-extended into the returned word, which is why this caller tests that word
 * with a bare bgez and no halfword sign extension of its own. */
s32 func_801AFF90(const void *a, const void *b, const void *c); /* @source 0x801AFF90 */

/* Projected screen entry of this overlay's draw passes: the projector writes the
 * halfword pair at +0x00/+0x02 and leaves the trailing four bytes unwritten.
 * The entry is eight bytes wide in this body's frame - the pair sits at 0x10 and
 * the projected point at 0x18, with the save block starting at 0x28. */
typedef struct BmagicMagic15003ScreenEntry {
  u16 x;
  u16 y;
  u16 unk_04;
  u16 unk_06;
} BmagicMagic15003ScreenEntry;

/* Effect transform of this overlay's own data run at 0x801EFA9C, published by
 * the overlay init 0x801EECA4 as the three words a record carries at
 * +0x34/+0x38/+0x3C, the first biased by 0x14000 and the third by 0x2C00000,
 * with its three halfwords at +0x10..+0x14 zeroed. This body reads the three
 * words as the point the projector consumes - x, y and z - and takes the signed
 * halfword at +0x10 as the ring radius, which the handler 0x801EEEC4 publishes
 * as 0x100 or 0x110 per frame parity and the adjacent handlers 0x801EEE24 and
 * 0x801EEF80 ramp by 0x40. 0x801EFAA8 (+0x0C) is written by nothing in this
 * overlay's image, and the burst draw 0x801EF4E0 reads the halfword at +0x12 as
 * a second radius, so that halfword stays unnamed here. */
typedef struct BmagicMagic15003EffectTransform {
  s32 x;
  s32 y;
  s32 z;
  s32 unk_0C;
  s16 radius;
} BmagicMagic15003EffectTransform;

extern BmagicMagic15003EffectTransform T_801EFA9C; /* @source 0x801EFA9C @kind data */

/* @source 0x801EF040
 * @behavior Draws this overlay's effect band as a ring of sixteen translucent
 *   flat quads around the effect transform at 0x801EFA9C.
 *   The phase starts at zero and each step advances it by 0x100 of the 0x1000
 *   circle, so sixteen steps close the ring. The station of a step is the phase
 *   truncated to an unsigned halfword and handed to both phase samplers, and its
 *   radial offset is the transform's signed radius times the cosine shifted
 *   right four on the y word and the same radius times the sine shifted left
 *   four on the z word, added to the transform's own y and z. The projected
 *   point therefore lies in the y-z plane at the transform's x, and the same
 *   point with 0x100000 added to x gives the far edge of the band.
 *   Every step takes the shared primitive cursor anew, seeds it through the
 *   flat-quad packet setter 0x8017A9A4 and turns semi-transparency on, then
 *   projects four points into one screen entry and copies each pair into the
 *   quad: vertex 0 is the near edge of the station at the current phase, vertex
 *   1 its far edge, vertex 2 the near edge at the phase one step further and
 *   vertex 3 that station's far edge. The phase is advanced between the two
 *   station pairs, so the second station is sampled after the increment. Only
 *   the red channel is stored, at 0x80, with green and blue cleared and the pad
 *   bytes of the colour word left as the previous packet had them.
 *   The step then passes the first three vertex pairs to the shared
 *   cross-product test 0x801AFF90 and appends the 0x18-byte packet at ordering
 *   table index 1 only while that test reads negative; a step whose test reads
 *   zero or positive leaves the cursor where it was, so that quad is dropped.
 *   The loop ends when the unsigned byte copy of the counter reaches sixteen.
 *   It takes no arguments - the only state it reads is the transform and the
 *   primitive cursor - and returns nothing.
 *   Name evidence: its only caller is the overlay draw setup 0x801EF254, which
 *   binds texture page (0, 1, 0x2C0, 0x100) with dither off, appends its 0x0C
 *   draw mode, primes the projector through 0x801AFE18 and then runs this body
 *   and the burst draw 0x801EF4E0 as the overlay's two draw passes, so the
 *   address is the overlay's ring pass rather than a handler of the 0x801EEC04
 *   table, whose seven words name other spans.
 *   Shape evidence: the projection scratch primed by the caller, the three-word
 *   point handed to 0x801AFF04, the cosine-right/sine-left radius pair and the
 *   phase-stepped quad walk are this repository's lifted family, e.g.
 *   drawTranslucentGouraudRingOfSixteenQuadsAroundWorkCursorWorld03Area14514_801F3980
 *   and this overlay's own burst draw 0x801EF4E0; those bodies carry their own
 *   counts, shades and packet types, so they are shape evidence only and every
 *   constant, shift, offset and helper binding here comes from these bytes.
 *   The clone lever was declined: no payload under out/binaries holds a window
 *   of these 532 bytes, compared word for word with the two transform-base words
 *   masked, and no window differs in eight words or fewer, so there is no twin
 *   to compare and no relocation-different sibling to seed from.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawTranslucentRedRingOfSixteenFlatQuadsAroundEffectOriginWhenSideNegativeBmagicMagic15003_801EF040(
    void) {
  BmagicMagic15003ScreenEntry screen;
  VECTOR point;
  BmagicMagic15003EffectTransform *transform;
  POLY_F4 *prim;
  s32 phase;
  u16 angle;
  s32 i;

  phase = 0;
  i = 0;
  transform = &T_801EFA9C;
  do {
    prim = (POLY_F4 *)g_PrimCursor;
    func_8017A9A4(prim);
    SetSemiTrans(prim, 1);
    angle = phase;
    point.vx = transform->x;
    point.vy = transform->y + ((transform->radius * func_801783C8(angle)) >> 4);
    point.vz = transform->z + ((transform->radius * func_801782FC(angle)) << 4);
    func_801AFF04(&point, &screen);
    prim->x0 = screen.x;
    prim->y0 = screen.y;
    point.vx = transform->x + 0x100000;
    func_801AFF04(&point, &screen);
    prim->x1 = screen.x;
    prim->y1 = screen.y;
    phase += 0x100;
    angle = phase;
    point.vx = transform->x;
    point.vy = transform->y + ((transform->radius * func_801783C8(angle)) >> 4);
    point.vz = transform->z + ((transform->radius * func_801782FC(angle)) << 4);
    func_801AFF04(&point, &screen);
    prim->x2 = screen.x;
    prim->y2 = screen.y;
    point.vx = transform->x + 0x100000;
    func_801AFF04(&point, &screen);
    prim->x3 = screen.x;
    prim->y3 = screen.y;
    prim->r0 = 0x80;
    prim->g0 = 0;
    prim->b0 = 0;
    if (func_801AFF90(&prim->x0, &prim->x1, &prim->x2) < 0) {
      GpuAppendPrim(1, 0x18);
    }
    i++;
  } while ((u8)i < 0x10);
}
