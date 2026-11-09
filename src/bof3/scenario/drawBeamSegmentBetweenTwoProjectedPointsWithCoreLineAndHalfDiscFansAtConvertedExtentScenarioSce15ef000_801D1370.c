#include "bof3/bof3.h"

extern u8* g_PrimCursor; /* @source 0x8014598C @kind bss */
extern s32 D_80143E6C;

void func_8014E5A0(u8 arg0, u8 arg1);
s32 func_8017A138(s32 arg0, s32 arg1);
void func_8017AA80(void* arg0);
void func_801AFE18(void* arg0);
void func_801AFF04(const void* arg0, void* arg1);
void func_801AFFD8(const void* arg0, void* arg1, void* arg2);
void func_801D1690(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5,
                   s32 arg6, s32 arg7);
void drawTranslucentFanOfEightGouraudTrianglesAroundOriginAtScaleFromStartAngleScenarioSce15ef000_801D1540(
    s16 x, s16 y, s32 scale, s32 start_angle);

/* One 8-byte screen entry of the projection scratch: the projector writes the
 * halfword pair at +0x00/+0x02 and this body reads back only that pair, so the
 * trailing four bytes stay unmodelled. */
typedef struct ScenarioSce15ef000ScreenPoint {
  u16 x;
  u16 y;
  u16 unk_04;
  u16 unk_06;
} ScenarioSce15ef000ScreenPoint;

/* @source 0x801D1370
 * @behavior Draws one translucent beam segment between two caller vertices. It
 * arms the primitive list the way this overlay's other emitters do, with
 * GetTPage(0, 1, 0x3C0, 0) as the texture page, SetDrawMode installing that page
 * on the packet published at g_PrimCursor with dfe 0 and dtd 1, func_8014E5A0
 * queuing the 0xC-byte DR_MODE under OT slot 3 and func_801AFE18 initialising
 * the 0x20-byte sort scratch at sp+0x30. It then emits the segment's core: a
 * 0x10-byte LINE_F2 at the cursor whose two corner pairs are the projections of
 * the two vertices through func_801AFF04, whose r0 is 0x40 with g0 and b0 zero,
 * and which is linked under OT slot 3 with that same size; its
 * semi-transparency flag is cleared rather than set. The segment axis is then
 * measured as the angle of the projected line, func_8017A138 with the y
 * difference as its first argument and the x difference as its second. The
 * caller's third argument arrives as an extent in the beam's own units: it is
 * stored into both halves of the input screen pair, func_801AFFD8 converts that
 * pair at the depth of the first vertex, and the converted halfword plus the
 * per-frame parity bit (D_80143E6C & 1) becomes the first radius, so the beam
 * narrows or widens with the distance of the vertex it is anchored on. A
 * half-disc fan of eight Gouraud triangles is then drawn at the first vertex
 * from that radius starting at the axis angle plus 0x400, the pair is converted
 * again at the second vertex for the second radius, a second fan is drawn at the
 * second vertex from the axis angle plus 0xC00, and func_801D1690 closes the
 * body by taking both vertex pairs, both radii and both start angles. The two
 * start angles sit half a turn apart, 0x800 of the 0x1000 phase circle, which is
 * the span each eight-triangle fan covers, so the two caps meet the body quad as
 * the round ends of one capsule.
 *
 * Evidence: the LINE_F2 identity is the 0x10 link size together with the
 * r0/g0/b0 stores at cursor offsets +0x04/+0x05/+0x06 and the two 4-byte corner
 * writes at +0x08 and +0x0C, the packet the exact sibling drawFlatLinePrim of
 * the battle module builds through the same seed func_8017AA80. The caller
 * drawZigzagOfFiveProjectedSegmentsScenarioSce15ef000_801D1914 hands two 16-byte
 * VECTOR locals and one signed halfword, and the callee
 * drawTranslucentFanOfEightGouraudTrianglesAroundOriginAtScaleFromStartAngleScenarioSce15ef000_801D1540
 * fixes the meaning of the derived pair: it takes the origin pair as signed
 * halfwords, the converted extent as a signed scale and the axis angle as the
 * phase it steps by 0x100 per triangle. The two fan calls pass the same input
 * pair, so one conversion per vertex is what the second func_801AFFD8 call
 * reuses, and the radius cell is read back with an unsigned halfword load before
 * the parity bit is added, then narrowed to a signed halfword by the
 * `sll 16`/`sra 16` pair. The address has in-payload `jal` sites in this overlay
 * (the two fan calls at 0x801D149C and 0x801D14E4 and the closing quad call at
 * 0x801D1510) and one in the zigzag entry, and no word in the payload holds it,
 * so the scenario runtime reaches the body only through those callers.
 *
 * Width: the third argument is a signed halfword, not a word. Both declared
 * widths measure the same instruction stream here because no statement extends
 * the value, but only the halfword declaration places the entry copy of that
 * parameter where the original keeps it, after the two literal words of the
 * GetTPage call: the word spelling measured 114 of 116 instructions with its
 * first difference at +0x14, where the copy sat two slots early. The signed
 * spelling is the one the same overlay's
 * drawTranslucentSectorFanOfThirtyTwoGouraudTrianglesAroundProjectedCenterAtConvertedExtentScenarioSce15ef000_801D1A20
 * gives the identical pair input it hands to func_801AFFD8.
 *
 * Levers: the data lever is declined because the range opens with the
 * `addiu $sp, $sp, -0x70` prologue, ends with `jr $ra` and holds two `jal` sites
 * of its own; the carve lever is declined because the span is one declared row
 * from this entry to the next declared entry (0x801D1540) with its own
 * prologue and return path; the clone lever is declined because both payload
 * addresses this body calls are this overlay's own, so any twin would differ in
 * those two absolute `jal` words alone.
 *
 * Frame: the 0x70-byte frame carries the 16-byte incoming argument home area at
 * sp+0x00, the four-word outgoing argument block of func_801D1690 from sp+0x10
 * (the call whose stack words are 0x10, 0x14, 0x18 and 0x1C), the two 8-byte
 * screen entries at sp+0x20 and sp+0x28, the 0x20-byte sort scratch from sp+0x30
 * and the saved group $s0-$s5 plus $ra from sp+0x50 to sp+0x6C.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawBeamSegmentBetweenTwoProjectedPointsWithCoreLineAndHalfDiscFansAtConvertedExtentScenarioSce15ef000_801D1370(
    VECTOR* first, VECTOR* second, s16 extent) {
  ScenarioSce15ef000ScreenPoint screen[2];
  u8 scratch[0x20];
  LINE_F2* prim;
  u16 tpage;
  s16 first_extent;
  s16 second_extent;
  s32 heading;
  s32 first_angle;
  s32 second_angle;

  tpage = GetTPage(0, 1, 0x3C0, 0);
  SetDrawMode((DR_MODE*)g_PrimCursor, 0, 1, tpage, 0);
  func_8014E5A0(3, 0xC);
  func_801AFE18(scratch);
  prim = (LINE_F2*)g_PrimCursor;
  func_8017AA80(prim);
  SetSemiTrans(prim, 0);
  func_801AFF04(first, &prim->x0);
  func_801AFF04(second, &prim->x1);
  prim->r0 = 0x40;
  prim->g0 = 0;
  prim->b0 = 0;
  func_8014E5A0(3, 0x10);
  heading = func_8017A138(prim->y1 - prim->y0, prim->x1 - prim->x0);
  screen[0].y = extent;
  screen[0].x = extent;
  func_801AFFD8(first, &screen[0], &screen[1]);
  first_angle = (u16)(heading + 0x400);
  first_extent = screen[1].x + (D_80143E6C & 1);
  drawTranslucentFanOfEightGouraudTrianglesAroundOriginAtScaleFromStartAngleScenarioSce15ef000_801D1540(
      prim->x0, prim->y0, first_extent, first_angle);
  func_801AFFD8(second, &screen[0], &screen[1]);
  second_angle = (u16)(heading + 0xC00);
  second_extent = screen[1].x + (D_80143E6C & 1);
  drawTranslucentFanOfEightGouraudTrianglesAroundOriginAtScaleFromStartAngleScenarioSce15ef000_801D1540(
      prim->x1, prim->y1, second_extent, second_angle);
  func_801D1690(prim->x0, prim->y0, first_extent, first_angle, prim->x1,
                prim->y1, second_extent, second_angle);
}
