#include "bof3/bof3.h"

extern u8* g_PrimCursor; /* @source 0x8014598C @kind bss */
void func_8014E5A0(u8 arg0, u8 arg1);
void func_801AFE18(void* arg0);
void drawTranslucentGouraudTriangleFromThreeIndexedRecordsScenarioScena0300_801F8200(
    u8* points, u8 first, u8 second, u8 third);

/* @source 0x801F8170
 * @behavior Draws one translucent Gouraud quad of this overlay's work object.
 * The caller hands the pointer cell 0x1F800044 plus 0x0C (all five in-payload
 * callers spell it that way), and the body arms the emitter the way this
 * overlay's drawing entry points do: GetTPage(0, 1, 0x3C0, 0) yields the
 * texture page, SetDrawMode installs it on the packet published at
 * g_PrimCursor with dfe 0 and dtd 1, func_8014E5A0 queues the 0xC-byte DR_MODE
 * under OT slot 1, and func_801AFE18 initialises the 0x20-byte sort scratch at
 * sp+0x18. It then calls the local
 * drawTranslucentGouraudTriangleFromThreeIndexedRecordsScenarioScena0300_801F8200
 * twice with that same pointer,
 * selecting the records indexed 0, 1 and 2 and then 0, 3 and 2, so the two
 * triangles fan across the four 16-byte vertices the work object carries from
 * offset 0x18 and share the diagonal 0-2. Each emission is a 0x1C-byte
 * POLY_G3 whose three vertex pairs are the projections of the selected
 * records and whose vertex colours are 0x40 grey, black and 0x80 grey, so the
 * published quad shades from the 0x40 record across the two dark corners to
 * the 0x80 one. Takes one argument and returns no value.
 *
 * Evidence: the packet identity is the helper's own store schedule - colour
 * triples at cursor +0x04/+0x05/+0x06, +0x0C/+0x0D/+0x0E and
 * +0x14/+0x15/+0x16 with the projected pairs at +0x08, +0x10 and +0x18,
 * func_8017A97C stamping the primitive and func_8014E5A0(1, 0x1C) linking it -
 * which is the POLY_G3 layout the exact func_801F8360 body and
 * drawTranslucentFanOfEightGouraudTrianglesAroundOriginAtScaleFromStartAngleScenarioSce15ef000_801D1540
 * build through the same seed, and the helper's index arithmetic
 * (record << 4) + 0xC is a 16-byte vertex stride. The draw-mode prologue is
 * word for word the one
 * drawBeamSegmentBetweenTwoProjectedPointsWithCoreLineAndHalfDiscFansAtConvertedExtentScenarioSce15ef000_801D1370
 * carries.
 *
 * Levers: the data lever is declined because the range opens with
 * `addiu $sp, $sp, -0x40`, ends with `jr $ra` and holds its own `jal` sites;
 * the carve lever is declined because the span is one declared row from this
 * entry to the next declared entry 0x801F8200 with its own prologue and return
 * path; the clone lever is declined because no payload under out/binaries
 * holds this 0x90-byte span, byte for byte, and none holds it with its two
 * in-payload 0x801F8200 `jal` words masked out either.
 *
 * Frame: the 0x40-byte frame carries the outgoing argument block up to the
 * SetDrawMode tw stack word at sp+0x10, the 0x20-byte sort scratch from
 * sp+0x18, the saved $s0 at sp+0x38 and $ra at sp+0x3C.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawTwoTranslucentGouraudTrianglesFromFourIndexedRecordsScenarioScena0300_801F8170(
    u8* work) {
  u8 scratch[0x20];
  u16 tpage;

  tpage = GetTPage(0, 1, 0x3C0, 0);
  SetDrawMode((DR_MODE*)g_PrimCursor, 0, 1, tpage, 0);
  func_8014E5A0(1, 0xC);
  func_801AFE18(scratch);
  drawTranslucentGouraudTriangleFromThreeIndexedRecordsScenarioScena0300_801F8200(
      work, 0, 1, 2);
  drawTranslucentGouraudTriangleFromThreeIndexedRecordsScenarioScena0300_801F8200(
      work, 0, 3, 2);
}
