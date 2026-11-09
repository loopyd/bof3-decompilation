#include "bof3/bof3.h"

/* Pointer cell of the work record this overlay publishes and every record
 * handler of the target reads. This body does not take the fan centre from it;
 * it reads work byte 6 to select the shade triple of the centre vertex and the
 * words at 0x18 and 0x1C as the fixed-point position pair handed to the
 * primitive linker.
 * @source 0x1F800044 @kind bss
 */
extern u8* D_1F800044;

/* Shared primitive cursor of the main executable, the primitive each of the
 * eight triangles is seeded in.
 * @source 0x8014598C @kind unknown
 */
extern u8* g_PrimCursor;

/* Shade triple of the overlay drawing tables at 0x801F6138: three consecutive
 * bytes per shade row, selected by work byte 6 times three, of which 0x801F6138
 * is the red member, 0x801F6139 the green and 0x801F613A the blue. The three
 * members bind at their own addresses, which are the three loads the table base
 * reaches, and the two far vertex shades of a triangle stay black.
 * @source 0x801F6138 @kind unknown
 */
extern u8 D_801F6138[];
/* @source 0x801F6139 @kind unknown */
extern u8 D_801F6139[];
/* @source 0x801F613A @kind unknown */
extern u8 D_801F613A[];

/* Shared primitive seeder of the main executable the gouraud-triangle emitters
 * of the payload call with the fresh cursor primitive; 0x8017A97C carries the
 * body type of this family, and the semi-transparency comes from the shared
 * SetSemiTrans beside it. */
void func_8017A97C(void* arg0);
/* Shared primitive linker of the main executable at 0x80155A08: the record's
 * fixed-point position pair, the ordering kind and the primitive size class. */
void func_80155A08(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
/* Shared rotator pair at 0x801782FC and 0x801783C8. This body takes the
 * 0x801783C8 sample for the x displacement and the 0x801782FC sample for the y
 * displacement of both spoke vertices. */
s32 func_801782FC(s32 arg0);
s32 func_801783C8(s32 arg0);

/* @source 0x801F4DF0
 * @behavior Overlay gouraud fan emitter of the payload. Its only in-image
 *           reference is the jal at 0x801F49A4 inside the record builder
 *           0x801F4798, which passes the record halfwords at 0xC and 0xE as the
 *           fan centre, the record halfword 0x28 plus the low bit of
 *           0x80143E6C as the signed amplitude and the record heading 0xC00
 *           further on as the start angle. It runs eight times over the shared
 *           primitive cursor g_PrimCursor, seeds each iteration as a POLY_G3
 *           through func_8017A97C and marks it semi-transparent, and lays one
 *           gouraud triangle whose first vertex is the centre and whose other
 *           two vertices are the rotator samples of the current angle and of
 *           the angle one 0x100 step further on, each sample scaled by the
 *           signed amplitude through a shift of 12. The centre shade is the
 *           table triple at 0x801F6138 + 3 * work byte 6, the two far shades
 *           are black, and func_80155A08 links the record's fixed-point
 *           position pair at 0x18/0x1C with kind 0 and the 0x1C size class. The
 *           angle advances 0x100 per triangle, so the eight triangles span a
 *           half turn. Takes centre x, centre y, amplitude and start angle,
 *           returns nothing, and keeps a 0x40-byte frame whose saved registers
 *           are $s0-$s6 and $ra.
 *
 * Name evidence: the body's own constants and loop bound fix the name - eight
 * iterations (`sltiu 8`), one POLY_G3 seeded per iteration and linked with the
 * 0x1C size class, two rotator samples per triangle one 0x100 step apart over a
 * 0x1000 turn (half turn over the loop), and the shade triple read as
 * 0x801F6138/9/A + 3 * work byte 6 with the two far vertex shades cleared.
 *
 * Frame evidence: the original reserves 0x40 and saves $s0-$s6 and $ra at
 *           0x20-0x3C, so 0x20 bytes below the saved area are never addressed.
 *           The first seed without them compiled to a 0x38 frame and measured
 *           49.17%; declaring the unreferenced 16-byte array below reproduced
 *           the 0x40 frame and took the same body to 88.03%. Of the 0x20 dead
 *           bytes, 0x10 is the outgoing-argument area every calling frame of
 *           this profile carries and 0x10 is the object declared below.
 *
 * Spelling evidence: the six far-shade bytes are stored through a volatile
 *           reference. Written as plain member stores the same six statements
 *           were measured reordered descending (0xE,0xD,0xC,0x16,0x15,0x14)
 *           against the original's ascending run; with the qualifier the run
 *           matches. The shade triple stays non-volatile: qualifying the three
 *           table arrays instead moved the shade run ahead of the green store
 *           and measured 103/118. The rotator argument is masked inline in all
 *           four samples rather than through a named phase local, which is what
 *           places the first `andi` after SetSemiTrans as the original has it;
 *           a named local measured 106/117.
 *
 * Shape evidence: no other payload carries these 468 bytes - the exact span and
 *           the same span with every jal and branch immediate masked both occur
 *           once across the 394 overlay payloads, so neither clone eligibility
 *           nor a byte-identical twin applies. The nearest exact sibling
 *           drawTranslucentGouraudFanOfEightTrianglesAroundWorkScreenPairTintedByWorkByteAWithJitteredRadiusBmagicMagic09703_801F083C
 *           supplies the declaration style, the POLY_G3 fan idiom and the four
 *           call shapes; its angle step is 0x200 and its centre comes from the
 *           work record, so it is shape evidence only.
 * @status partial
 * @match 96.61
 * @residual Three instructions are scheduled differently and one load-delay
 *           slot stays a nop, so the body is 0x1D8 bytes against the original
 *           0x1D4. The original fills the delay slots of the third work-pointer
 *           load and of the work-byte-6 load with the loop increment and the
 *           materialised kind constant; this body schedules both after the
 *           gate-4 sample instead. Measured alternatives that leave the same
 *           residual class: increment before the zero run, increment inside a
 *           `for` header, increment in the loop condition (which breaks the
 *           loop shape instead), plain member stores, chained stores, the shade
 *           run ahead of the blue store, and volatile shade tables. Everything
 *           else - registers $s0-$s6, both call shapes, all nine vertex stores,
 *           the ascending shade run and the four rotator calls - agrees.
 */
void drawTranslucentEightTriangleGouraudFanAroundScreenPairTintedByWorkByte6TripleWorld03Area14813_801F4DF0(
    s32 arg0, s32 arg1, s32 arg2, s32 arg3) {
  u8 unusedStack[16];
  POLY_G3* prim;
  s16 amp;
  u8 i;

  i = 0;
  amp = arg2;
  do {
    prim = (POLY_G3*)g_PrimCursor;
    func_8017A97C(prim);
    SetSemiTrans(prim, 1);
    prim->x0 = arg0;
    prim->y0 = arg1;
    prim->x1 = arg0 + ((amp * func_801783C8(arg3 & 0xFFFF)) >> 12);
    prim->y1 = arg1 + ((amp * func_801782FC(arg3 & 0xFFFF)) >> 12);
    arg3 += 0x100;
    prim->x2 = arg0 + ((amp * func_801783C8(arg3 & 0xFFFF)) >> 12);
    prim->y2 = arg1 + ((amp * func_801782FC(arg3 & 0xFFFF)) >> 12);
    prim->r0 = D_801F6138[D_1F800044[6] * 3];
    prim->g0 = D_801F6139[D_1F800044[6] * 3];
    prim->b0 = D_801F613A[D_1F800044[6] * 3];
    FIELD_REF(volatile u8, prim, 0xCu) = 0;
    FIELD_REF(volatile u8, prim, 0xDu) = 0;
    FIELD_REF(volatile u8, prim, 0xEu) = 0;
    FIELD_REF(volatile u8, prim, 0x14u) = 0;
    FIELD_REF(volatile u8, prim, 0x15u) = 0;
    FIELD_REF(volatile u8, prim, 0x16u) = 0;
    func_80155A08(FIELD_REF(s32, D_1F800044, 0x18u),
                  FIELD_REF(s32, D_1F800044, 0x1Cu), 0, 0x1C);
    i++;
  } while (i < 8);
}
