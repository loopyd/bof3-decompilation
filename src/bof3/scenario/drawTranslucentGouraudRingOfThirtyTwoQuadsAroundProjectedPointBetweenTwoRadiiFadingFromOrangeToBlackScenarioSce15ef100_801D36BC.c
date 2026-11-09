#include "bof3/bof3.h"

/* Primitive cursor cell of the shared main executable. */
extern u8* g_PrimCursor; /* @source 0x8014598C @kind bss */

/* Resident helpers of the shared main executable this emitter reaches: the
 * primitive appender 0x8014E5A0, the 4096-unit cosine and sine samplers
 * 0x801783C8 and 0x801782FC, the sort-scratch initialiser 0x801AFE18 and the
 * point projector 0x801AFF04, declared with the prototypes this payload's other
 * lifts use. */
void func_8014E5A0(u8 arg0, u8 arg1);
s32 func_801782FC(s32 arg0);
s32 func_801783C8(s32 arg0);
void func_801AFE18(void* arg0);
void func_801AFF04(const void* arg0, void* arg1);

/* One 8-byte screen entry of the projection output: the projector writes the
 * halfword pair at +0x00/+0x02 and this body reads back only that pair, so the
 * trailing four bytes stay unmodelled. The pair is 8 bytes because the two
 * entries the body keeps sit at frame offsets 0x28 and 0x30. */
typedef struct ScenarioSce15ef100ScreenEntry {
  u16 x;
  u16 y;
  u16 unk_04;
  u16 unk_06;
} ScenarioSce15ef100ScreenEntry;

/* @source 0x801D36BC
 * @behavior Translucent Gouraud quad-ring emitter. It arms the primitive list
 * the way this payload's other fan emitters do: GetTPage(0, 1, 0x2C0, 0x100)
 * yields the texture page, SetDrawMode installs that page on the packet
 * published at g_PrimCursor with dfe 0 and dtd 0, func_8014E5A0 queues the
 * 0x0C-byte DR_MODE under OT slot 2, and func_801AFE18 initialises the 0x20-byte
 * sort scratch at sp+0x38. It then seeds the phase-zero corner pair: the
 * caller's three-word point with the orange radius shifted left four (the
 * 0x1000-unit cosine at phase zero divided by the shift of eight) added to its
 * x is projected through func_801AFF04 into the first screen entry, and the same
 * point with the black radius shifted left four is projected into the second.
 * The loop runs 32 unsigned-byte iterations of one semi-transparent POLY_G4
 * emission at the primitive cursor. Each pass stamps the packet through
 * SetPolyG4, marks it translucent, writes vertices 0 and 1 from the two corners
 * the previous pass left in the entries, advances the phase by 0x80 of the
 * 4096-unit turn, recomputes the corners at that new phase into vertices 2 and
 * 3, and appends the 0x24-byte packet through func_8014E5A0 under OT slot 2.
 * Neighbouring quads share their edges and the thirty-two of them close exactly
 * one turn. Every corner is the caller point translated by a radius rotated in
 * the x/y plane, cosine for x and sine for y with the product shifted right by
 * eight, and the translated point is re-projected for each corner, so the ring
 * follows the caller point in world space at its own depth. Vertices 0 and 2
 * carry the orange triplet 0xFF/0x7F/0x00 and vertices 1 and 3 stay black, so
 * the band runs from the orange rim built from the first radius to the black rim
 * built from the second. It takes the three-word point in $a0 and the two radii
 * in $a1 and $a2 and returns nothing.
 *
 * Evidence: the only in-payload call sites are the two step handlers this
 * target lifts exact at 0x801D2C00 and 0x801D2D5C, and each hands the work
 * object's point at +0x34 with the work object's own word pair at +0x0C/+0x10
 * as the two radii, so the argument roles come from the callers and the widths
 * from the loads. The vector is the PsyQ 16-byte VECTOR: the body writes only
 * its vx/vy/vz words at frame 0x18/0x1C/0x20 and leaves the pad word at 0x24
 * untouched, which is what makes the two screen entries land at 0x28 and 0x30
 * instead of 0x24. The preamble constants 0x2C0/0x100 with abr 1, the (2, size)
 * append class, the 0x24-byte POLY_G4 stores at 0x04 through 0x22 and the
 * func_8014E5A0 arming are the ones the exact sibling
 * drawTranslucentSectorFanOfThirtyTwoGouraudTrianglesAroundProjectedCenterAtConvertedExtentScenarioSce15ef000_801D1A20
 * of the neighbouring sce15ef0 overlay shares word for word, and the rising
 * colour-store order and the phase seed are the ones the exact sibling
 * drawTranslucentGouraudRingOfSixteenQuadsAroundScreenCenterFadingFromBlackToWhiteScenarioScena0800_801F9688
 * records for the same quad body. The shift of eight follows the original sra
 * on the sampler product, which the
 * phase-zero seed of (radius << 4) corroborates (0x1000 >> 8).
 *
 * Levers: the data lever is declined because the span opens with the 0x80-byte
 * frame setup and closes with the ra restore and jr ra, and the carve lever
 * because the span is one declared row ending exactly at the next declared entry
 * 0x801D3918. The clone lever is declined with the equality already measured:
 * the only byte-identical span in the reviewed payloads is
 * emi/world02/area098/13 at payload offset 0x1890, and that target's row is
 * still a raw asm row with no source to clone, so it is a shape witness and a
 * duplicate lead for the parent rather than a seed.
 *
 * Frame: the 0x80-byte frame carries the outgoing stack argument of SetDrawMode
 * at sp+0x10, the 0x18-byte VECTOR at sp+0x18, the two 8-byte screen entries at
 * sp+0x28 and sp+0x30, the 0x20-byte sort scratch from sp+0x38 to sp+0x57 and
 * the saved group $s0-$s7, $fp and $ra from sp+0x58 to sp+0x7C.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawTranslucentGouraudRingOfThirtyTwoQuadsAroundProjectedPointBetweenTwoRadiiFadingFromOrangeToBlackScenarioSce15ef100_801D36BC(
    s32* point, s32 orangeRadius, s32 blackRadius) {
  VECTOR                       vertex;
  ScenarioSce15ef100ScreenEntry screen[2];
  u8                           scratch[0x20];
  POLY_G4*                     prim;
  u16                          tpage;
  u16                          angle;
  s32                          turn;
  u8                           i;

  tpage = GetTPage(0, 1, 0x2C0, 0x100);
  SetDrawMode((DR_MODE*)g_PrimCursor, 0, 0, tpage, 0);
  func_8014E5A0(2, 0xC);
  func_801AFE18(scratch);
  vertex.vx = point[0] + (orangeRadius << 4);
  vertex.vy = point[1];
  vertex.vz = point[2];
  func_801AFF04(&vertex, &screen[0]);
  vertex.vx = point[0] + (blackRadius << 4);
  func_801AFF04(&vertex, &screen[1]);
  turn = 0;
  i = 0;
  do {
    prim = (POLY_G4*)g_PrimCursor;
    SetPolyG4(prim);
    SetSemiTrans(prim, 1);
    prim->x0 = screen[0].x;
    prim->y0 = screen[0].y;
    prim->x1 = screen[1].x;
    prim->y1 = screen[1].y;
    turn += 0x80;
    angle = turn;
    vertex.vx = point[0] + ((orangeRadius * func_801783C8(angle)) >> 8);
    vertex.vy = point[1] + ((orangeRadius * func_801782FC(angle)) >> 8);
    func_801AFF04(&vertex, &screen[0]);
    vertex.vx = point[0] + ((blackRadius * func_801783C8(angle)) >> 8);
    vertex.vy = point[1] + ((blackRadius * func_801782FC(angle)) >> 8);
    func_801AFF04(&vertex, &screen[1]);
    prim->x2 = screen[0].x;
    prim->y2 = screen[0].y;
    prim->x3 = screen[1].x;
    prim->y3 = screen[1].y;
    prim->r0 = 0xFF;
    prim->g0 = 0x7F;
    prim->b0 = 0x00;
    prim->r1 = 0x00;
    prim->g1 = 0x00;
    prim->b1 = 0x00;
    prim->r2 = 0xFF;
    prim->g2 = 0x7F;
    prim->b2 = 0x00;
    prim->r3 = 0x00;
    prim->g3 = 0x00;
    prim->b3 = 0x00;
    func_8014E5A0(2, 0x24);
    i++;
  } while (i < 0x20);
}
