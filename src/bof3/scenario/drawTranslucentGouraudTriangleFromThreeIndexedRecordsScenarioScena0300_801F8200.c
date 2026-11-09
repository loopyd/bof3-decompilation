#include "bof3/bof3.h"
#include "gpu/prim.h"

/* Resident projector: it reads the three-word point handed at arg0 and writes
 * the projected screen pair at arg1, the role the exact scenario siblings give
 * it. */
void func_801AFF04(const void* arg0, void* arg1); /* @source 0x801AFF04 */

/* Resident primitive seed helper: it stamps the packet it receives before the
 * colour and vertex fields of that packet are filled in. */
void func_8017A97C(void* arg0); /* @source 0x8017A97C */

/* @source 0x801F8200
 * @behavior Draws one translucent Gouraud triangle of this overlay's work
 * object. The received pointer is the work object plus 0x0C, so the vertex
 * array it addresses starts at +0x0C with a 16-byte stride, which is why each
 * selection is `(index << 4) + 0xC` off that pointer. The three argument bytes
 * select three of those entries, and each selected entry's three-word point is
 * projected by func_801AFF04 into the packet published at g_PrimCursor: the
 * first selection into the pair at +0x08, the second into +0x10 and the third
 * into +0x18, the three vertex pairs of a 0x1C-byte POLY_G3. func_8017A97C
 * stamps that packet and SetSemiTrans marks it semi-transparent, then the
 * colour triplet at +0x04 takes 0x40 grey and the one at +0x14 takes 0x80
 * grey while the middle triplet at +0x0C stays black, so the triangle shades
 * from the 0x40 first vertex across the dark second vertex to the 0x80 third.
 * func_8014E5A0(1, 0x1C) links the finished packet under allocation class 1.
 * Takes the work pointer plus three record indices and returns no value.
 *
 * Evidence: the packet identity is this body's own store schedule - colour
 * triples at cursor +0x04/+0x05/+0x06, +0x0C/+0x0D/+0x0E and
 * +0x14/+0x15/+0x16 around the projected pairs at +0x08, +0x10 and +0x18,
 * func_8017A97C stamping the primitive and func_8014E5A0(1, 0x1C) linking it -
 * which is the POLY_G3 layout its only in-payload caller, the exact
 * drawTwoTranslucentGouraudTrianglesFromFourIndexedRecordsScenarioScena0300_801F8170,
 * describes from the same offsets. That caller passes its own `work` argument
 * through unchanged and indexes records 0, 1, 2 and 0, 3, 2 of the four
 * 16-byte vertices the work object carries from offset 0x18, so the 0x0C this
 * body adds is the start of that array and (index << 4) is its element stride;
 * func_801AFF04 is the shared projector the exact scenario siblings hand a
 * three-word point, which fits those 16-byte entries. No payload under
 * out/binaries names 0x801F8200 as a data word, so this caller is its only
 * route.
 *
 * Levers: the data lever is declined because the range opens with
 * `addiu $sp, $sp, -0x28`, ends with `jr $ra` and holds its own eight `jal`
 * sites; the carve lever is declined because the span is one declared row from
 * this entry to the next declared entry 0x801F82E4 with its own prologue and
 * return path; the clone lever is declined because a scan of every payload
 * under out/binaries finds this 0xE4-byte span only in its own payload, and
 * none at all with its eight `jal` immediates masked, which is the conclusive
 * form here because every callee and the g_PrimCursor cell resolve in the
 * shared main executable rather than in this payload.
 *
 * Frame: the 0x28-byte frame holds $s0 at sp+0x10, $s1 at sp+0x14, $s2 at
 * sp+0x18, $s3 at sp+0x1C and $ra at sp+0x20; the body needs no outgoing
 * argument slot because its widest call passes two registers.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawTranslucentGouraudTriangleFromThreeIndexedRecordsScenarioScena0300_801F8200(
    u8* points, u8 first, u8 second, u8 third) {
  u8* p0;
  u8* p1;
  u8* p2;
  u8* prim;

  p0 = points + ((first << 4) + 0xC);
  p1 = points + ((second << 4) + 0xC);
  p2 = points + ((third << 4) + 0xC);
  prim = g_PrimCursor;

  func_8017A97C(prim);
  SetSemiTrans(prim, 1);
  func_801AFF04(p0, prim + 8);
  func_801AFF04(p1, prim + 0x10);
  func_801AFF04(p2, prim + 0x18);
  prim[4] = 0x40;
  prim[5] = 0x40;
  prim[6] = 0x40;
  prim[0xC] = 0;
  prim[0xD] = 0;
  prim[0xE] = 0;
  prim[0x14] = 0x80;
  prim[0x15] = 0x80;
  prim[0x16] = 0x80;
  GpuAppendPrim(1, 0x1C);
}
