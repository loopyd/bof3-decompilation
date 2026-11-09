#include "bof3/bof3.h"

/* Pointer cell of the work record this overlay publishes and every record
 * handler of the target reads. This body reads work byte 6 to select the shade
 * triple of the two near vertices and the words at 0x18 and 0x1C as the
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

/* Shade triple of the overlay drawing tables at 0x801F6138: three consecutive
 * bytes per shade row, selected by work byte 6 times three, of which 0x801F6138
 * is the red member, 0x801F6139 the green and 0x801F613A the blue. The three
 * members bind at their own addresses, which are the three loads the table base
 * reaches, and the four far vertex shades of a quad stay black.
 * @source 0x801F6138 @kind unknown
 */
extern u8 D_801F6138[];
/* @source 0x801F6139 @kind unknown */
extern u8 D_801F6139[];
/* @source 0x801F613A @kind unknown */
extern u8 D_801F613A[];

/* Shared primitive linker of the main executable at 0x80155A08: the record's
 * fixed-point position pair, the ordering kind and the primitive size class.
 */
void func_80155A08(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
/* Shared rotator pair at 0x801782FC and 0x801783C8. This body takes the
 * 0x801783C8 sample for the x displacement and the 0x801782FC sample for the y
 * displacement of every spoke of both quads. */
s32 func_801782FC(s32 arg0);
s32 func_801783C8(s32 arg0);

/* @source 0x801F4A50
 * @behavior Overlay gouraud quad emitter of the payload. Its only in-image
 *           reference is the jal at 0x801F49D8 inside the record builder
 *           0x801F4798, which passes the record halfwords at 0x8 and 0xA as the
 *           first screen pair, the recorded amplitude and the accumulated angle
 *           plus 0x400 as its signed amplitude and its angle, the record
 *           halfwords at 0xC and 0xE as the second screen pair, the record
 *           halfword 0x28 plus the low bit of 0x80143E6C as the second signed
 *           amplitude and the accumulated angle plus 0xC00 as the second angle.
 *           The body seeds one POLY_G4 in the shared primitive cursor
 *           g_PrimCursor through SetPolyG4, marks it semi-transparent, writes
 *           the two screen pairs as its first two vertices and lays its two far
 *           vertices on the rotator samples 0x801783C8 and 0x801782FC of the two
 *           angles, each sample scaled by one signed amplitude through a shift
 *           of 12: the first far vertex leaves the first screen pair along the
 *           first angle with the first amplitude, the second leaves the second
 *           screen pair along the second angle plus a half turn with the second
 *           amplitude. The two near vertices take the shade triple at
 *           0x801F6138 + 3 * work byte 6 and the four far vertex shades are
 *           black. func_80155A08 then links the record fixed-point position pair
 *           at 0x18/0x1C with kind 0 and the 0x24 size class. The 0x24-byte
 *           primitive is copied one size class further on and its two far
 *           vertices are rewritten as the same pair of spokes turned half a
 *           turn round - the first screen pair along the first angle plus 0x800
 *           with the first amplitude, the second screen pair along the bare
 *           second angle with the second amplitude - and the copy is linked by a
 *           second func_80155A08 call with the same position pair, kind 0 and
 *           0x24 size class.
 *
 * Name evidence: the body's own constants and calls fix the name - one
 *           SetPolyG4 seed per quad, a 0x24-byte object copied to the next size
 *           class, two quads linked with kind 0 and the 0x24 size class, two
 *           screen pairs and two rotator samples per quad over a 0x1000 turn
 *           with the 0x800 half turn separating the two quads, and the shade
 *           triple read as 0x801F6138/9/A + 3 * work byte 6 with the four far
 *           vertex shades cleared.
 *
 * Spelling evidence: the six vertex inputs are u16 locals, which is what makes
 *           the body store and reload them as halfwords at 8-byte-strided slots
 *           and zero-extend them into the vertex sums; the two amplitudes are
 *           s16 locals, which is what makes each use a shift pair rather than a
 *           halfword reload. The two amplitudes stay separate locals because
 *           the original multiplies the first amplitude into the two spokes of
 *           the first screen pair and the second amplitude into the two spokes
 *           of the second screen pair in both quads.
 *
 * Shape evidence: no other payload carries these 928 bytes - neither the exact
 *           span nor the same span with every jump and branch immediate masked
 *           occurs anywhere in the 395 other captured payload binaries, so
 *           neither clone eligibility nor a byte-identical twin applies. The
 *           exact sibling drawTranslucentEightTriangleGouraudFanAroundScreenPairTintedByWorkByte6TripleWorld03Area14813_801F4DF0
 *           supplies the declaration style, the shade triple idiom and the four
 *           call shapes; it emits eight POLY_G3 triangles from one screen pair
 *           over the same record, so it is shape evidence only.
 * @status partial
 * @match 74.79
 * @residual The body is 0x3A8 bytes against the original 0x3A0: the original
 *           keeps both amplitude parameters in the callee-saved registers $s6
 *           and $s7 and spills the second angle local to the 0x38 slot, while
 *           this body keeps the second angle in $s7 and spills the raw third
 *           parameter to a slot at 0x10. That one spilled allocno reserves an
 *           extra 8-byte slot, moves the six vertex slots up by 8 and emits the
 *           store/reload pair, while the frame, the shade run, the copy loop,
 *           all eight rotator calls and both linker calls agree instruction for
 *           instruction. Measured alternatives that leave the same residual
 *           class: both amplitude locals declared before the vertex locals,
 *           the amplitudes assigned at the top of the body, before their first
 *           use, and after the vertex stores, the vertex locals assigned in
 *           reversed pairs, the second angle declared first or last, the vertex
 *           inputs typed as u16 parameters instead of locals (which measures
 *           948 bytes), and a named `angle + 0x800` local per quad (944 and 948
 *           bytes). The two shapes that reach the original instruction count
 *           and 0x3A0 bytes spill the same parameter and mismatch on the
 *           prologue register saves.
 */
void drawTranslucentTwoGouraudQuadsAroundScreenPairTintedByWorkByte6TripleWorld03Area14813_801F4A50(
    s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5, s32 arg6,
    u16 arg7) {
  POLY_G4* prim;
  u16 x0, y0, ang0, x1, y1, ang1;
  s16 amp0, amp1;

  prim = (POLY_G4*)g_PrimCursor;
  SetPolyG4(prim);
  SetSemiTrans(prim, 1);
  x0 = arg0;
  y0 = arg1;
  ang0 = arg3;
  x1 = arg4;
  y1 = arg5;
  ang1 = arg7;
  amp0 = arg2;
  amp1 = arg6;
  prim->x0 = x0;
  prim->y0 = y0;
  prim->x1 = x1;
  prim->y1 = y1;
  prim->x2 = x0 + ((amp0 * func_801783C8(ang0)) >> 12);
  prim->y2 = y0 + ((amp0 * func_801782FC(ang0)) >> 12);
  prim->x3 = x1 + ((amp1 * func_801783C8(ang1 + 0x800)) >> 12);
  prim->y3 = y1 + ((amp1 * func_801782FC(ang1 + 0x800)) >> 12);
  prim->r0 = D_801F6138[D_1F800044[6] * 3];
  prim->g0 = D_801F6139[D_1F800044[6] * 3];
  prim->b0 = D_801F613A[D_1F800044[6] * 3];
  prim->r1 = D_801F6138[D_1F800044[6] * 3];
  prim->g1 = D_801F6139[D_1F800044[6] * 3];
  prim->b1 = D_801F613A[D_1F800044[6] * 3];
  prim->r2 = 0;
  prim->g2 = 0;
  prim->b2 = 0;
  prim->r3 = 0;
  prim->g3 = 0;
  prim->b3 = 0;
  func_80155A08(FIELD_REF(s32, D_1F800044, 0x18u),
                FIELD_REF(s32, D_1F800044, 0x1Cu), 0, 0x24);
  prim++;
  *prim = prim[-1];
  prim->x2 = x0 + ((amp0 * func_801783C8(ang0 + 0x800)) >> 12);
  prim->y2 = y0 + ((amp0 * func_801782FC(ang0 + 0x800)) >> 12);
  prim->x3 = x1 + ((amp1 * func_801783C8(ang1)) >> 12);
  prim->y3 = y1 + ((amp1 * func_801782FC(ang1)) >> 12);
  func_80155A08(FIELD_REF(s32, D_1F800044, 0x18u),
                FIELD_REF(s32, D_1F800044, 0x1Cu), 0, 0x24);
}
