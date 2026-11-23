#include "bof3/bof3.h"

/* Primitive cursor cell of the shared main executable. */
extern u8* g_PrimCursor; /* @source 0x8014598C @kind bss */

/* Shared frame counter of the main executable; only its low bit is read here. */
extern s32 D_80143E6C;

/* Primitive submit helper of the shared main executable: it takes the ordering
 * table slot and the primitive size as its two byte arguments. */
void func_8014E5A0(u8 arg0, u8 arg1);

/* Shared phase samplers: both return the sample of a 12-bit phase in the same
 * fixed-point scale as the >> 12 products below. This body takes the 0x801783C8
 * sample for the x offsets and the 0x801782FC sample for the y offsets. */
s32 func_801782FC(s32 arg0);
s32 func_801783C8(s32 arg0);

/* Shared engine primitive stamper, called on the fresh cursor primitive before
 * this body writes its fields. */
void func_8017A97C(void* arg0);

/* Projection setup helper of the shared main executable: it fills the 0x20-byte
 * scratch block handed in $a0. */
void func_801AFE18(void* arg0);

/* Point projector of the shared main executable: it projects the three-word
 * point handed in arg0 into the screen entry handed in arg1. */
void func_801AFF04(const void* arg0, void* arg1);

/* Size-pair converter of the shared main executable: it converts the size pair
 * handed in arg1 at the depth of the point handed in arg0 and leaves the
 * converted pair in arg2. */
void func_801AFFD8(const void* arg0, void* arg1, void* arg2);

/* One 8-byte screen entry of the projection scratch: the two projectors write
 * the halfword pair at +0x00/+0x02 and this body reads back only that pair, so
 * the trailing four bytes stay unmodelled. */
typedef struct ScenarioSce15ef200ScreenPoint {
  u16 x;
  u16 y;
  u16 unk_04;
  u16 unk_06;
} ScenarioSce15ef200ScreenPoint;

/* @source 0x801D1218
 * @behavior Draws one opaque radial sector fan around a projected point. It arms
 * the primitive list the way this overlay's other emitters do, GetTPage(0, 1,
 * 0x2C0, 0x100) for the texture page, SetDrawMode installing that page on the
 * packet published at g_PrimCursor with dfe 0, dtd 1 and a zero fifth argument,
 * func_8014E5A0 queuing the 0xC-byte DR_MODE under OT slot 7, and func_801AFE18
 * initialising the 0x20-byte sort scratch at sp+0x38. It then projects the
 * caller's three-word origin point through func_801AFF04 into the first screen
 * entry, copies the caller's signed halfword into both halves of the third
 * entry and hands that pair with the same point to func_801AFFD8, the shared
 * size-pair converter, so the fourth entry holds the extent converted at the
 * point's own depth, which is the radius this fan draws with. The low bit of the
 * shared frame counter is added to that radius before use, so the fan breathes
 * by one sub-pixel unit on alternating frames, and the second entry is seeded
 * with the phase-zero arc point: the projected x plus the whole radius and the
 * projected y unchanged. The original reads the radius cell with an unsigned
 * halfword load for that sum and with a signed halfword load for each rotation
 * product, so the cell stays an unsigned halfword here and the two products
 * carry the signed conversion their `mult` operands require.
 *
 * The loop runs 32 unsigned-byte iterations of one opaque POLY_G3 emission at
 * the primitive cursor. Vertex 0 is the projected point and vertex 1 the
 * phase-zero arc point seeded before the loop, so every triangle is the circular
 * sector from angle zero to its own phase; vertex 2 is rebuilt each pass as the
 * projected point plus the radius rotated by the phase, with func_801783C8
 * giving the x offset and func_801782FC the y offset and both products shifted
 * down by twelve, the 0x1000-unit turn and the fixed-point radius those helpers
 * and the shipped callers imply. The phase advances by 0x80 per pass, so the fan
 * closes exactly one turn at the thirty-second triangle. The triangle's first
 * colour triplet takes 0x80 in a channel for each low bit of the caller's mask,
 * red for bit 2, green for bit 1 and blue for bit 0, and its other two triplets
 * stay black, so the fan is lit only at the shared centre. Each packet is
 * appended under OT slot 7 with the 0x1C-byte POLY_G3 size through
 * func_8014E5A0.
 *
 * Name evidence: the only in-image reference to 0x801D1218 is the `jal` at
 * 0x801D0C58 in the overlay entry func_801D0C04, which selects the run of work
 * handlers with the work byte at +0x1 and then hands this span the work object's
 * record at +0x34, the signed halfword at +0xC through `lh` and the literal 7,
 * so the mask lights all three channels at once and no other site names this
 * address. Name roles come from this body's own literals and helper calls as
 * described above.
 *
 * Shape evidence: the sibling overlays emi/scenario/sce15ef0/00 and
 * emi/scenario/sce15ef1/00 carry this body at 0x801D1A20 and 0x801D147C, and
 * the sce15ef1 lift,
 * drawTranslucentSectorFanOfThirtyTwoGouraudTrianglesAroundProjectedCenterAtConvertedExtentScenarioSce15ef100_801D147C,
 * is exact. One `cmp` of the two 0x1FC-byte payload spans shows they differ in
 * exactly three words: the two OT-slot immediates of the func_8014E5A0 calls,
 * 7 here against 2 there, and the SetSemiTrans argument, where this span's
 * `addu $a1, $zero, $zero` clears the semi-transparency bit that the sibling sets
 * with `addiu $a1, $zero, 0x1`. Every other word, including all `jal`
 * relocations, both `%hi`/`%lo` pairs and every branch, is equal, so the sibling
 * body is the model for this one and this function's own literals come from its
 * own bytes. That is shape evidence rather than clone eligibility, because the
 * spans are not byte-identical.
 *
 * Frame: the original 0x78-byte frame carries the outgoing stack argument of
 * SetDrawMode at sp+0x10, four 8-byte screen entries from sp+0x18 to sp+0x37, the
 * 0x20-byte sort scratch from sp+0x38 to sp+0x57 and the saved group $s0..$s6
 * plus $ra from sp+0x58 to sp+0x74. The four entries are visible in the halfword
 * offsets the body addresses, 0x18/0x1A, 0x20/0x22, 0x28/0x2A and 0x30/0x32, so
 * one address-taken array of that entry type keeps them together and in memory
 * across the calls.
 * @status exact
 * @match 100.00
 * @residual none
 */
void renderOpaqueSectorConverted(
    const void* origin, s16 extent, s32 channel_mask) {
  ScenarioSce15ef200ScreenPoint screen[4];
  u8 scratch[0x20];
  POLY_G3* prim;
  u16 tpage;
  u16 phase;
  u16 radius;
  u8 red;
  u8 green;
  u8 blue;
  u8 i;
  s32 turn;

  tpage = GetTPage(0, 1, 0x2C0, 0x100);
  SetDrawMode((DR_MODE*)g_PrimCursor, 0, 1, tpage, 0);
  func_8014E5A0(7, 0xC);
  func_801AFE18(scratch);
  func_801AFF04(origin, &screen[0]);
  screen[2].y = extent;
  screen[2].x = extent;
  func_801AFFD8(origin, &screen[2], &screen[3]);
  red = (channel_mask & 4) << 5;
  green = (channel_mask & 2) << 6;
  blue = (channel_mask & 1) << 7;
  radius = (u16)(screen[3].x + (D_80143E6C & 1));
  screen[3].x = radius;
  screen[1].x = (u16)(screen[0].x + radius);
  screen[1].y = screen[0].y;
  turn = 0;
  i = 0;
  do {
    prim = (POLY_G3*)g_PrimCursor;
    func_8017A97C(prim);
    SetSemiTrans(prim, 0);
    prim->x0 = screen[0].x;
    prim->y0 = screen[0].y;
    prim->x1 = screen[1].x;
    prim->y1 = screen[1].y;
    turn += 0x80;
    phase = (u16)turn;
    screen[1].x =
        (u16)(screen[0].x + (((s16)screen[3].x * func_801783C8(phase)) >> 12));
    screen[1].y =
        (u16)(screen[0].y + (((s16)screen[3].x * func_801782FC(phase)) >> 12));
    prim->x2 = screen[1].x;
    prim->y2 = screen[1].y;
    prim->r0 = red;
    prim->g0 = green;
    prim->b0 = blue;
    prim->r1 = 0;
    prim->g1 = 0;
    prim->b1 = 0;
    prim->r2 = 0;
    prim->g2 = 0;
    prim->b2 = 0;
    i++;
    func_8014E5A0(7, 0x1C);
  } while (i < 0x20);
}
