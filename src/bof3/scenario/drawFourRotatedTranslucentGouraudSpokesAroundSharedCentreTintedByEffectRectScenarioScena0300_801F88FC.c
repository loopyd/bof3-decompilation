#include "bof3/bof3.h"
#include "gpu/prim.h"

/* Shared phase samplers: the fan emitter declared next in this overlay takes
 * the 0x801783C8 sample for a vertex x and the 0x801782FC sample for its y,
 * each product shifted down twelve, so this body reads them as the
 * cosine-like and sine-like samples of a 12-bit phase. Its three products
 * shift down eight instead, because the record scale it multiplies is already
 * shifted down eight. */
extern s32 func_801782FC(s32 phase); /* @source 0x801782FC */
extern s32 func_801783C8(s32 phase); /* @source 0x801783C8 */

/* Shared resident primitive seed helper: it stamps the packet it receives
 * before this body fills that packet's colour and vertex fields, the position
 * the eight-segment line burst of world00/area067/14 also gives it. */
extern void func_8017AA94(void* arg0); /* @source 0x8017AA94 */

/* Shared resident projection helper: it consumes the 16-byte word triple the
 * caller fills and writes the projected halfword pair to the second argument,
 * which is why this body keeps one VECTOR local and moves its second argument
 * from packet +0x8 to packet +0x10 between the two vertices. */
extern void func_801AFF04(void* arg0, void* arg1); /* @source 0x801AFF04 */

/* Shared resident sort-scratch initialiser: the quad emitter of this overlay
 * passes it the same 0x20-byte stack scratch this body reserves. */
extern void func_801AFE18(void* arg0); /* @source 0x801AFE18 */

/* Shared word triple the projected vertices are offset from. The target map
 * does not name it and the generated undefined-symbol table already binds the
 * D_ spelling, so the base is declared here at its fixed address and read as
 * three consecutive words; this body reuses one base register for all three
 * offsets. */
extern s32 D_80145EC4;

/* @source 0x801F88FC
 * @behavior Draws the four rotated radial spokes of the effect-rectangle
 * record the caller hands in as work plus 0x0C. The prologue arms the emitter
 * the way this overlay's drawing entry points do: GetTPage(0, 1, 0x3C0, 0)
 * yields the texture page, SetDrawMode installs it on the packet published at
 * g_PrimCursor with dfe 0 and dtd 1, GpuAppendPrim queues the 0xC-byte DR_MODE
 * under OT slot 1, and func_801AFE18 initialises the 0x20-byte sort scratch.
 * Before the loop it builds the spoke direction as one VECTOR triple:
 * func_801782FC of record halfword +0x2 times func_801783C8 of record halfword
 * +0x4 shifted down eight for x, that first sample times func_801782FC of +0x4
 * shifted down eight for y, and func_801783C8 of +0x2 shifted up four for z.
 * Each of the four passes reloads g_PrimCursor, stamps that packet with
 * func_8017AA94 and marks it semi-transparent, then projects two vertices
 * through func_801AFF04. The first vertex is the shared word triple at
 * 0x80145EC4, 0x80145EC8 and 0x80145ECC plus that direction scaled by the
 * signed product of record halfword +0x0 with halfword +0x6 (width) shifted
 * down eight, projected into the packet pair at +0x8; the second uses the same
 * direction scaled by the product of +0x0 with halfword +0x8 (height),
 * projected into the pair at +0x10. The packet colours are the record bytes
 * +0xA, +0xB and +0xC scaled the same way, by width for the first vertex at
 * +0x4 to +0x6 and by height for the second at +0xC to +0xE, and GpuAppendPrim
 * links the finished 0x14-byte Gouraud line under OT slot 1. The direction is
 * then rotated a quarter turn about its z axis by (x, y) to (-y, x) as the
 * pass count advances, so the four spokes sit at right angles to one another.
 * Takes the effect-rectangle record and returns no value.
 *
 * Evidence: this 0x308-byte span is the payload's own; the byte comparison
 * that declined the clone lever found it nowhere else. The record layout, the
 * direction arithmetic and the two-vertex packet layout come from the
 * original bytes above, and the emitter prologue, the packet field offsets and
 * the argument roles follow the exact siblings
 * drawTwoTranslucentGouraudTrianglesFromFourIndexedRecordsScenarioScena0300_801F8170
 * and
 * drawTranslucentGouraudFanOfThirtyTwoTrianglesAroundScreenPointAtRadiusTintedByCenterAndRimScenarioScena0300_801F8C04,
 * which arm the same texture page, link through the same OT slot and shift the
 * same phase products down twelve. The single in-payload `jal` to this entry
 * is inside
 * drawAndShrinkWorkHalfwordsThenAdvanceScenarioScena0300_801F853C, which
 * passes the work pointer cell 0x1F800044 plus 0x0C, so the effect-rectangle
 * record seeded by
 * seedEffectRectWithRandomOffsetsAndAdvanceWorkByte1ScenarioScena0300_801F8490
 * is the argument: intensity at +0x0, the two random phase halfwords at +0x2
 * and +0x4, width at +0x6, height at +0x8 and the colour bytes at +0xA, +0xB
 * and +0xC. No word or table entry in the payload holds this entry as a
 * callback.
 *
 * Levers: the data lever is declined because the range opens with
 * `addiu $sp, $sp, -0x80`, ends with `jr $ra` and holds eight `jal` sites; the
 * carve lever is declined because the span is one declared row that runs from
 * this entry to the next declared entry 0x801F8C04 with its own prologue and
 * return path, and its declared size 0x308 ends exactly on that entry; the
 * clone lever is declined because no payload under out/binaries holds these
 * 776 bytes, byte for byte, and the span needs no masking to compare since
 * every absolute word it addresses lies outside the payload.
 *
 * Frame: the 0x80-byte frame carries the SetDrawMode fifth argument at sp+0x10,
 * the projected VECTOR at sp+0x18, the running direction VECTOR at sp+0x28, its
 * pre-rotation copy at sp+0x38, the 0x20-byte sort scratch from sp+0x48, the
 * saved $s0 at sp+0x68, $s1 at sp+0x6C, $s2 at sp+0x70, $s3 at sp+0x74 and $ra
 * at sp+0x78.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawFourRotatedTranslucentGouraudSpokesAroundSharedCentreTintedByEffectRectScenarioScena0300_801F88FC(
    u8* rect) {
  VECTOR offset;
  VECTOR step;
  VECTOR previous;
  u8 scratch[0x20];
  s32* centre;
  u16 tpage;
  u8* primitive;
  s32 scale;
  s8 i;

  tpage = GetTPage(0, 1, 0x3C0, 0);
  SetDrawMode((DR_MODE*)g_PrimCursor, 0, 1, tpage, 0);
  GpuAppendPrim(1, 0xC);
  centre = &D_80145EC4;
  i = 0;
  step.vx = (func_801782FC(FIELD_REF(s16, rect, 0x2u)) *
             func_801783C8(FIELD_REF(s16, rect, 0x4u))) >>
            8;
  step.vy = (func_801782FC(FIELD_REF(s16, rect, 0x2u)) *
             func_801782FC(FIELD_REF(s16, rect, 0x4u))) >>
            8;
  step.vz = func_801783C8(FIELD_REF(s16, rect, 0x2u)) << 4;
  func_801AFE18(scratch);
  do {
    primitive = g_PrimCursor;
    func_8017AA94(primitive);
    SetSemiTrans(primitive, 1);
    scale =
        (s16)((FIELD_REF(s16, rect, 0x0u) * FIELD_REF(s16, rect, 0x6u)) >> 8);
    offset.vx = centre[0] + ((scale * step.vx) >> 8);
    offset.vy = centre[1] + ((scale * step.vy) >> 8);
    offset.vz = centre[2] + ((scale * step.vz) >> 8);
    func_801AFF04(&offset, primitive + 8);
    primitive[4] = (FIELD_REF(s16, rect, 0x6u) * FIELD_REF(u8, rect, 0xAu)) >> 8;
    primitive[5] = (FIELD_REF(s16, rect, 0x6u) * FIELD_REF(u8, rect, 0xBu)) >> 8;
    primitive[6] = (FIELD_REF(s16, rect, 0x6u) * FIELD_REF(u8, rect, 0xCu)) >> 8;
    scale =
        (s16)((FIELD_REF(s16, rect, 0x0u) * FIELD_REF(s16, rect, 0x8u)) >> 8);
    offset.vx = centre[0] + ((scale * step.vx) >> 8);
    offset.vy = centre[1] + ((scale * step.vy) >> 8);
    offset.vz = centre[2] + ((scale * step.vz) >> 8);
    func_801AFF04(&offset, primitive + 0x10);
    primitive[0xC] =
        (FIELD_REF(s16, rect, 0x8u) * FIELD_REF(u8, rect, 0xAu)) >> 8;
    primitive[0xD] =
        (FIELD_REF(s16, rect, 0x8u) * FIELD_REF(u8, rect, 0xBu)) >> 8;
    primitive[0xE] =
        (FIELD_REF(s16, rect, 0x8u) * FIELD_REF(u8, rect, 0xCu)) >> 8;
    GpuAppendPrim(1, 0x14);
    i = i + 1;
    previous = step;
    step.vx = -previous.vy;
    step.vy = previous.vx;
  } while (i < 4);
}
