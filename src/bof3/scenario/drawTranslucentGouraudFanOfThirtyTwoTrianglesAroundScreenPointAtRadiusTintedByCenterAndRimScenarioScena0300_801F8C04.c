#include "bof3/bof3.h"
#include "gpu/prim.h"

/* Resident primitive seed helper: it stamps the packet it receives before the
 * colour and vertex fields of that packet are filled in. */
extern void func_8017A97C(void* arg0); /* @source 0x8017A97C */

/* Resident rotation pair: the exact fan emitters of this build multiply each
 * sample by one of these and shift the product down twelve, the 0x1000-unit
 * turn the callers' phase step and radius scale imply. */
extern s32 func_801782FC(s32 phase); /* @source 0x801782FC */
extern s32 func_801783C8(s32 phase); /* @source 0x801783C8 */

/* @source 0x801F8C04
 * @behavior Emits 32 semi-transparent Gouraud triangles as one closed radial
 * fan around the caller's screen pair. The prologue arms the emitter the way
 * this overlay's drawing entry points do: GetTPage(0, 1, 0x3C0, 0) yields the
 * texture page, SetDrawMode installs it on the packet published at
 * g_PrimCursor with dfe 0 and dtd 1, and GpuAppendPrim queues the 0xC-byte
 * DR_MODE under OT slot 1. The loop then reloads g_PrimCursor each pass,
 * stamps that packet with func_8017A97C and marks it semi-transparent, and
 * fills one 0x1C-byte POLY_G3 whose first vertex is the caller's screen pair
 * in the fourth argument's colour and whose other two vertices are the radius
 * samples at the running angle and at that angle plus 0x80, both in the fifth
 * argument's colour: func_801783C8 gives the x offset and func_801782FC the y
 * offset, each product shifted down twelve. The radius is taken as a signed
 * halfword, the angle advances by 0x80 per pass over a 0x1000-unit turn so the
 * fan closes on the thirty-second triangle, and the wrapped second sample is
 * the next pass's first sample. GpuAppendPrim links each finished packet under
 * OT slot 1 at its 0x1C-byte size. Takes the screen pair, the radius and the
 * center and edge colours and returns no value.
 *
 * Evidence: this 0x1A4-byte span is byte-identical to the span at offset 0x304
 * of out/binaries/emi/world00/area032/13.bin, whose source
 * src/bof3/world/func_801F2F04.c stood at status exact when this seed was
 * written, and to offset 0x1190 of out/binaries/emi/world02/area108/14.bin,
 * which was still an asm row. Each comparison is one `cmp` of the extracted
 * 420-byte spans.
 * The statement order, operand forms, offset spelling and declaration style
 * are that exact sibling's, the width of the screen pair included: the signed
 * halfword pair is what puts x in $s8 and y in $s7, where a word pair swaps
 * those two registers and leaves the other 95 instructions equal. This span's
 * own literals, packet offsets and registration order come from the original
 * bytes above. The three in-payload callers fix the argument roles: each hands
 * the scratchpad work pair at +0x2E/+0x30 as the screen point, signed work
 * byte 9 as the radius, 0x80 as the center colour and 0 as the edge colour.
 *
 * Levers: the data lever is declined because the range opens with
 * `addiu $sp, $sp, -0x40`, ends with `jr $ra` and holds nine `jal` sites; the
 * carve lever is declined because the span is one declared row from this entry
 * to the next declared entry 0x801F8DA8 with its own prologue and return path;
 * the clone lever is used, as the byte comparison above proves, and its tool
 * dry-run was unavailable because the representative source includes a
 * target-local internal header.
 *
 * Frame: the 0x40-byte frame carries the SetDrawMode tw stack word at sp+0x10,
 * the saved $s0..$s7 and $fp from sp+0x18 to sp+0x38 and $ra at sp+0x3C.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawTranslucentGouraudFanOfThirtyTwoTrianglesAroundScreenPointAtRadiusTintedByCenterAndRimScenarioScena0300_801F8C04(
    s16 x, s16 y, s16 radius, u8 center, u8 edge) {
  s32 temp_lo_1;
  s32 temp_lo_2;
  s32 wrapped;
  s32 next;
  s32 angle;
  u8* primitive;

  SetDrawMode((DR_MODE*)g_PrimCursor, 0, 1, GetTPage(0, 1, 0x3C0, 0), 0);
  GpuAppendPrim(1, 0xC);
  angle = 0;
  do {
    primitive = g_PrimCursor;
    func_8017A97C(primitive);
    SetSemiTrans(primitive, 1);
    do {
      next = angle + 0x80;
    } while (0);
    wrapped = next & 0xFFF;
    {
      u16 current = angle;

      *(s16*)(primitive + 8) = x;
      *(s16*)(primitive + 0xA) = y;
      *(s16*)(primitive + 0x10) = x + ((radius * func_801783C8(current)) >> 12);
      *(s16*)(primitive + 0x12) = y + ((radius * func_801782FC(current)) >> 12);
    }
    temp_lo_1 = radius * func_801783C8((u16)wrapped);
    angle = next;
    *(s16*)(primitive + 0x18) = x + (temp_lo_1 >> 12);
    next = func_801782FC((u16)wrapped);
    temp_lo_2 = radius * next;
    primitive[4] = center;
    primitive[5] = center;
    primitive[6] = center;
    primitive[0xC] = edge;
    primitive[0xD] = edge;
    primitive[0xE] = edge;
    primitive[0x14] = edge;
    primitive[0x15] = edge;
    primitive[0x16] = edge;
    *(s16*)(primitive + 0x1A) = y + (temp_lo_2 >> 12);
    GpuAppendPrim(1, 0x1C);
  } while ((u16)angle < 0x1000);
}
