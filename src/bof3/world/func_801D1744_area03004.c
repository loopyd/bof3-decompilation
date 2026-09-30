#include "bof3/world/area03004_internal.h"

/* @behavior builds the AREA030 16x32 Gouraud quad in place at the shared
 * primitive cursor g_PrimCursor: SetPolyG4 stamps the tag, the upper vertex row
 * becomes (arg0, arg1) and (arg0 + 0x10, arg1) with the bright 0x64/0x64/0x80
 * colour pair, the lower vertex row repeats those x positions (read back from
 * the quad) one 0x20 row down with the dim 0x00/0x00/0x50 pair, and
 * GpuAppendPrim then appends the 0x24-byte quad under the caller graphics-mode
 * byte arg2.
 * @source 0x801D1744
 * @status partial
 * @match 83.02
 * @residual live `bin/asm-diff emi/world00/area030/04@0x801d1744` is 44/53
 * insns with the size and instruction count exact (212 bytes, 53/53) and the
 * first difference at +0x0034: the original hoists the GpuAppendPrim
 * byte-argument mask `andi a0,s2,0xff` ahead of `move v0,s1` at the head of the
 * post-call block and keeps the x1 store/read-back pair (`sh s1,16(s0)`,
 * `lhu a2,16(s0)`) adjacent at +0x0040, while this build emits the same
 * instruction multiset with that mask at +0x0084 and the x1 pair at +0x0080.
 * The multiset is identical apart from two register picks (the x1 read-back
 * lands in $v1 rather than $a2 and the 0x50 colour constant in $v0 rather than
 * $v1), so the residual is scheduler/allocation order. Measured and rejected
 * clean-C shapes (all 44/53 or worse, many bit-identical to this build):
 * statement-order permutations of the position stores and read-backs, chained
 * colour assignments (34/53), per-channel colour grouping (41/53),
 * s16/u16/u32/s32 locals for the read-back values or the position parameters,
 * pointer-declaration initializer, explicit `arg2 & 0xFF`, `sizeof(POLY_G4)`,
 * and a top-of-function mode local. Next untried rung is the opt-in
 * compiler-profile probe (flag-search / per-object flags), which this mission
 * excludes.
 */
void func_801D1744(s16 arg0, s16 arg1, u8 arg2) {
  POLY_G4* primitive;

  primitive = (POLY_G4*)g_PrimCursor;
  SetPolyG4(primitive);
  primitive->x0 = arg0;
  primitive->x1 = arg0 + 0x10;
  primitive->y0 = arg1;
  primitive->r0 = 0x64;
  primitive->g0 = 0x64;
  primitive->b0 = 0x80;
  primitive->r1 = 0x64;
  primitive->g1 = 0x64;
  primitive->b1 = 0x80;
  primitive->y1 = arg1;
  primitive->r2 = 0x00;
  primitive->g2 = 0x00;
  primitive->b2 = 0x50;
  primitive->r3 = 0x00;
  primitive->g3 = 0x00;
  primitive->b3 = 0x50;
  primitive->x2 = primitive->x0;
  primitive->x3 = primitive->x1;
  primitive->y3 = primitive->y2 = primitive->y0 + 0x20;
  GpuAppendPrim(arg2, 0x24);
}
