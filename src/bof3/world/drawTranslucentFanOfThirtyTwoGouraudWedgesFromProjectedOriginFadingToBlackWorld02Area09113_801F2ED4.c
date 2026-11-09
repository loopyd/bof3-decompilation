#include "bof3/bof3.h"
#include "gpu/prim.h"

/* Fixed scratchpad vertex triplet at 0x1F800014. The original materialises its
 * address once with a `lui` plus `addiu` pair into $s2 and then addresses the
 * three vertices as offsets 0x0, 0x8 and 0x10 of that base, so the cell is
 * declared as one object here and indexed. The exact bmagic sibling
 * drawFourPhasedTrianglesFromOriginTintedByWorkRecordLifetime at 0x801EF5F4
 * binds the same cell the same way. The target's generated
 * undefined_syms_auto.txt already names the address, so no target-map row is
 * needed. */
extern SVECTOR D_1F800014; /* @source 0x1F800014 @kind data */

/* Shared screen matrix the overlay transform is composed under. The address is
 * owned by this target's generated undefined_syms_auto.txt row
 * D_801492E8 = 0x801492E8. Every access hands it to MulMatrix2 as a matrix
 * base, which the exact bmagic sibling
 * projectWorkRecordPositionAndInstallTransform at 0x801EF52C shows for the same
 * cell. */
extern MATRIX D_801492E8; /* @source 0x801492E8 @kind unknown */

/* Shared phase samplers reached through this payload's generated
 * undefined_funcs_auto.txt rows; both return a 12-bit fixed-point sample. */
s32 func_801782FC(s32 arg0);
s32 func_801783C8(s32 arg0);

/* Graphic-type query feeding the draw-mode texture page argument. */
s32 GetGraphType(void);

/* Shared primitive linker: binds the primitive at g_PrimCursor to the passed
 * ordering-table words, tags it with the passed kind and advances the cursor by
 * the passed size class, which is why this body links one 0xC-byte packet once
 * per call and then one 0x1C-byte packet per wedge. */
void func_80155A08(s32 arg0, s32 arg1, s32 arg2, s32 arg3);

/* @source 0x801F2ED4
 * @behavior Overlay radial shade fan drawn under a transform of its own
 *           making. The body pushes the GTE matrix stack, fills a fixed
 *           position triplet (-0x3040, -0x3300, 0x80) and a zeroed rotation
 *           triplet, projects that position into the translation field of a
 *           0x20-byte scratch matrix through RotTrans while storing its flag
 *           word beside the matrix, turns the zeroed rotation into the same
 *           matrix through RotMatrix, composes the shared screen matrix at
 *           0x801492E8 under it through MulMatrix2 and publishes the result
 *           through SetRotMatrix and SetTransMatrix, so the fan below is
 *           emitted around that projected fixed point instead of the local
 *           origin. It then picks the draw-mode texture page 0xAE when
 *           GetGraphType returns 1 or 2 and 0x3E otherwise, arms SetDrawMode on
 *           the primitive cursor g_PrimCursor with both dither arguments zero,
 *           links the 0xC-byte packet through func_80155A08 with the fixed
 *           ordering words 0x1F8000 and 0x1A0000 and kind 2, and stamps
 *           thirty-two wedges of one semi-transparent POLY_G3 apiece.
 *           Each wedge keeps its first vertex at the scratchpad origin triplet
 *           0x1F800014, which the body zeroes on every pass, and takes its two
 *           arc vertices from the samplers func_801783C8 for x and
 *           func_801782FC for y scaled by the first argument truncated to an
 *           unsigned halfword and shifted right twelve. The wedge phase starts
 *           at zero and the counter advances by one between the two arc
 *           vertices, so a wedge spans the phases counter<<7 and the following
 *           one and neighbouring wedges meet at the shared sample, thirty-two
 *           steps of 0x80 closing the 0x1000 turn. Before projecting, the body
 *           sets the packet length byte to 6 and the colour-0 code byte to
 *           0x30 and marks the packet semi-transparent; RotTransPers3 then
 *           writes the three projected vertex pairs at packet offsets
 *           0x08/0x10/0x18, the first colour triple takes the second argument
 *           in red and green with a zero blue and the two later triples go
 *           black, which fades the fan from the caller-supplied shade at the
 *           projected origin to black at the arc, and the packet is linked
 *           through func_80155A08 as a 0x1C-byte class before the next pass.
 *           The matrix stack is popped after the thirty-second pass.
 *           Takes a radius and a shade and returns nothing. The 0x80-byte frame
 *           keeps $ra and $s0-$s6, the sixteen-byte outgoing argument area at
 *           0x10, the position triplet at 0x20, the rotation triplet at 0x28,
 *           the scratch matrix at 0x30 whose translation field is the RotTrans
 *           output, and the RotTrans flag word at 0x50 plus the two
 *           RotTransPers3 output words at 0x54 and 0x58.
 *           The area091 sibling copyHalvedPalettePairOnCountdownAndAdvanceRow
 *           at 0x801F3318 is the only in-payload caller and passes 0x12C and
 *           0xFF, so the fan is drawn at radius 300 with the full centre shade.
 *           Shape evidence, not clone proof: bytes 0x801F2ED4 through 0x801F312B
 *           match no payload under out/binaries, and the exact lifts
 *           projectWorkRecordPositionAndInstallTransform at 0x801EF52C (the
 *           RotTrans into the matrix translation field, RotMatrix and MulMatrix2
 *           chain) and drawFourPhasedTrianglesFromOriginTintedByWorkRecordLifetime
 *           at 0x801EF5F4 (the same scratchpad triplet, samplers, POLY_G3 colour
 *           triples and 0x1C-byte link) carry the two halves of this body at
 *           different addresses with different constants, so this source was
 *           seeded from the original bytes.
 * Live evidence: 131 of 150 instructions and 600 of 600 bytes, with the first
 *           divergence at the preheader counter initialization. Frame size,
 *           saved-register set, stack offsets, outgoing argument area and every
 *           constant and call word agree; the 19 divergent instructions are the
 *           scratchpad vertex base at 0x1F800014 and the wedge counter holding
 *           each other's callee-saved register (current $s1/$s2 against the
 *           original's $s2/$s1), which renames the same instruction sequence
 *           rather than reordering it.
 * @status partial
 * @match 87.33
 * @residual Register assignment of two loop values only. Allocation evidence from
 *           the owning compiler (bin/cc, the -O2 -G0 -funsigned-char -msoft-float
 *           profile of compile_commands.json) puts the vertex base at 25
 *           references over 77 instructions and the counter at 11 over 79 in the
 *           flow data, so global-alloc ranks the base first and gives it the
 *           lower callee-saved register; the original groups those same thirteen
 *           base references differently. Spelling probes that were measured and
 *           reverted: inline i << 7 for the phase (strength-reduced to a phase
 *           accumulator, 592 bytes), a phase accumulator free pointer form, an
 *           array-typed cell, a declaration-time base initializer, masked radius
 *           copies, and declaration-order permutations, each of which left the
 *           same first divergence or regressed. The permuter rung is untried and
 *           needs parent authorization.
 */
void drawTranslucentFanOfThirtyTwoGouraudWedgesFromProjectedOriginFadingToBlackWorld02Area09113_801F2ED4(
    s32 arg0, s32 arg1) {
  SVECTOR pos;
  SVECTOR rot;
  MATRIX mtx;
  long trans_flag;
  long depth;
  long flag;
  s32 angle;
  SVECTOR *v;
  POLY_G3 *prim;
  s32 tpage;
  s32 i;
  s32 ot_x;
  s32 ot_y;

  PushMatrix();
  pos.vx = -0x3040;
  pos.vy = -0x3300;
  rot.vx = 0;
  rot.vy = 0;
  rot.vz = 0;
  pos.vz = 0x80;
  RotTrans(&pos, (VECTOR *)mtx.t, &trans_flag);
  RotMatrix(&rot, &mtx);
  MulMatrix2(&D_801492E8, &mtx);
  SetRotMatrix(&mtx);
  SetTransMatrix(&mtx);
  ot_x = 0x1F8000;
  ot_y = 0x1A0000;
  if (GetGraphType() == 1 || GetGraphType() == 2) {
    tpage = 0xAE;
  } else {
    tpage = 0x3E;
  }
  SetDrawMode((DR_MODE *)g_PrimCursor, 0, 0, tpage, 0);
  i = 0;
  v = &D_1F800014;
  func_80155A08(ot_x, ot_y, 2, 0xC);
  arg0 = arg0 & 0xFFFF;
  do {
    v[0].vx = 0;
    v[0].vy = 0;
    v[0].vz = 0;
    angle = i << 7;
    v[1].vx = (arg0 * func_801783C8(angle)) >> 12;
    v[1].vy = (arg0 * func_801782FC(angle)) >> 12;
    v[1].vz = 0;
    i++;
    angle = i << 7;
    v[2].vx = (arg0 * func_801783C8(angle)) >> 12;
    v[2].vy = (arg0 * func_801782FC(angle)) >> 12;
    v[2].vz = 0;
    prim = (POLY_G3 *)g_PrimCursor;
    setlen(prim, 6);
    prim->code = 0x30;
    SetSemiTrans(prim, 1);
    RotTransPers3(v, v + 1, v + 2, (long *)&prim->x0, (long *)&prim->x1,
                  (long *)&prim->x2, &depth, &flag);
    prim->r0 = arg1;
    prim->g0 = arg1;
    prim->b0 = 0;
    prim->r1 = 0;
    prim->g1 = 0;
    prim->b1 = 0;
    prim->r2 = 0;
    prim->g2 = 0;
    prim->b2 = 0;
    func_80155A08(ot_x, ot_y, 2, 0x1C);
  } while (i < 0x20);
  PopMatrix();
}
