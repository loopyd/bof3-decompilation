#include "bof3/bof3.h"
#include "gpu/prim.h"

/* Work-record pointer cell at the tail of this overlay data block, the last
 * word before the next segment at 0x801F1058. The dispatcher at 0x801EF340, the
 * handler at 0x801EF384 and the exact sibling
 * projectWorkRecordPositionAndInstallTransformBmagicMagic06303_801EF994 all read
 * the same cell, and the record it points into is one of the 0x60 0x20-byte
 * entries of the pool at 0x801F0454, so the cell holds a live record when this
 * function runs. Splat binds the address through this target generated
 * undefined_syms_auto.txt row, so no map row is needed. The cell is read once by
 * the opening func_80155A08 for both of its cell words, once for the three
 * colour products, once more inside the loop after the primitives those
 * products feed have been emitted, and once by the closing func_80155A08, so it
 * is bound by name rather than kept in a local. */
extern u8 *D_801F1054; /* @source 0x801F1054 @kind data */

/* Scratchpad halfword cell at 0x1F80000A, the low cell of the three-word colour
 * triplet this body publishes. The original forms its address as one `lui $a1`
 * plus `addiu $a1` pair and keeps it live, because the scalar block below is
 * addressed as offsets of it (0x1F800000 at -0xA). The two higher triplet cells
 * are published through literal constants instead, the absolute `lui $at` plus
 * `sh` form the compiler emits for a direct fixed-address store. */
extern u16 D_1F80000A; /* @source 0x1F80000A @kind data */

/* Scratchpad halfword cell at 0x1F800014, the first vertex of the triangle this
 * body projects. Its address is formed the same way and kept live because the
 * two later vertices sit at offsets 0x8 and 0x10 of it, while its own two
 * trailing halfwords are cleared through literal constants. */
extern u16 D_1F800014; /* @source 0x1F800014 @kind data */

/* Four-entry halfword phase tables inside the 0x801F0400 data run: the first
 * two hold the two vertex radii, the second two the two vertex phases, each
 * entry indexed by the loop counter. They are laid out end to end with an
 * eight-byte stride, and this target payload holds 0x0060/0x0030, 0x0060/0x0030,
 * 0x0030/0x0080 and 0x0030/0x0080 in them, the same contents the exact-relocation
 * magic106 sibling holds, so the phase shape transfers with this target's own
 * addresses. The trailing byte table at 0x801F043C is the per-phase z jitter
 * added to work-record byte 0x0B before it is masked to four bits. */
extern u16 D_801F041C[]; /* @source 0x801F041C @kind data */
extern u16 D_801F0424[]; /* @source 0x801F0424 @kind data */
extern u16 D_801F042C[]; /* @source 0x801F042C @kind data */
extern u16 D_801F0434[]; /* @source 0x801F0434 @kind data */
extern u8 D_801F043C[];  /* @source 0x801F043C @kind data */

/* Shared grid-cell primitive linker: it resolves the (x, y) cell pair, links a
 * primitive at g_PrimCursor with the given kind and advances the cursor by the
 * given size class, which is why this body links a 0xC-byte packet once and
 * then one 0x1C-byte packet per triangle. */
void func_80155A08(s32 arg0, s32 arg1, s32 arg2, s32 arg3);

/* Shared primitive seed helper: it sits between the cursor read and
 * SetSemiTrans, the position the exact bmagic siblings give their own seed
 * helper. */
void func_8017A97C(void *arg0);

/* Shared phase samplers: both return the sample of a 12-bit phase in the same
 * fixed-point scale as the `>> 12` products below. */
s32 func_801782FC(s32 arg0);
s32 func_801783C8(s32 arg0);

/* Shared graphic-type query used for the draw-mode tpage argument. */
s32 GetGraphType(void);

/* @source 0x801EFA5C
 * @behavior Battle-magic drawing step of the record published through the
 *           pointer cell at 0x801F1054. It is reached by the two `jal` sites
 *           0x801EF3EC in the handler at 0x801EF384, which pushes the matrix
 *           stack and pops it after this call, and 0x801EFDE4 in the driver at
 *           0x801EFD7C, so it runs inside an already installed transform. It
 *           first picks the draw-mode tpage argument, 0xA5 when GetGraphType
 *           returns 1 or 2 and 0x35 otherwise, seeds SetDrawMode at
 *           g_PrimCursor with dfe 0 and dtd 1, and links the 0xC-byte cell at
 *           the record words 0x10 and 0x14 with kind 2. It then publishes the
 *           three products of the record unsigned bytes 0x05, 0x06 and 0x07
 *           with byte 0x09 into the scratchpad halfwords 0x1F80000A, 0x1F80000C
 *           and 0x1F80000E, the colour triplet, and builds the scalar block at
 *           0x1F800000 from the four halfword tables plus a phase of the record
 *           byte 0x0B added to the per-phase byte table and masked to four bits
 *           and shifted left eight, while the first triangle vertex at
 *           0x1F800014 and its two trailing halfwords at 0x1F800016 and
 *           0x1F800018 are cleared. The body runs four iterations; each takes a
 *           fresh g_PrimCursor primitive, seeds it through func_8017A97C and
 *           marks it semi-transparent, fills the two later vertices from the
 *           func_801782FC and func_801783C8 samples of the phase cells scaled by
 *           the radius cells and shifted right 12, with the shared z of the
 *           func_801782FC sample of the jitter cell shifted right 7, projects the
 *           three vertices through RotTransPers3 into the primitive vertex pairs
 *           at 0x08/0x10/0x18 with the two result words at $sp+0x20 and
 *           $sp+0x24, writes the colour bytes so that the first vertex takes the
 *           triplet in b/g/r order and the two later ones in r/g/b order, and
 *           links the 0x1C-byte cell at the same record words. So each iteration
 *           emits one triangle running from the origin through the two
 *           tabulated radii and phases, and the four phases form the burst.
 *           Takes no arguments, returns nothing, keeps a 0x40-byte frame for
 *           $ra, $s3, $s2, $s1 and $s0 plus the fifth draw-mode argument at
 *           0x10 and the two RotTransPers3 output words at 0x20 and 0x24, and
 *           writes no state outside the scratchpad cells and the primitives it
 *           emits. Bytes 0x801EFA5C through 0x801EFD7B are identical to the
 *           exact bmagic magic106 sibling drawFourPhasedTrianglesFromOrigin at
 *           0x801EF5F4 in every one of the 200 words but nine low halves of data
 *           cell loads, which carry 0x1F1054/0x1F041C/0x1F0424/0x1F042C/0x1F0434/
 *           0x1F043C here and 0x1F05F8/0x1EF9D4/0x1EF9DC/0x1EF9E4/0x1EF9EC/0x1EF9F4
 *           there, plus the one internal branch encoding that moves with the
 *           segment base, so that relocation-different sibling is shape evidence
 *           only and this source is a transcription carrying this target's own
 *           cells. No payload under out/binaries/emi holds this 800-byte body
 *           byte for byte, so the clone lever is ineligible.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawFourPhasedTrianglesFromOriginTintedByWorkRecordLifetimeBmagicMagic06303_801EFA5C(
    void) {
  u16 *tint;
  s16 *work;
  struct ScratchScalar { s16 radius0; };
  SVECTOR *shape;
  SVECTOR *mid;
  POLY_G3 *prim;
  long depth;
  long flag;
  s32 graph_value;
  s32 z;
  u8 i;

  if (GetGraphType() == 1 || GetGraphType() == 2) {
    graph_value = 0xA5;
  } else {
    graph_value = 0x35;
  }
  SetDrawMode((DR_MODE *)g_PrimCursor, 0, 1, graph_value, 0);
  func_80155A08(*(s32 *)(D_801F1054 + 0x10), *(s32 *)(D_801F1054 + 0x14), 2,
                0xC);

  tint = &D_1F80000A;
  *tint = D_801F1054[5] * D_801F1054[9];
  *(u16 *)0x1F80000C = D_801F1054[6] * D_801F1054[9];
  *(u16 *)0x1F80000E = D_801F1054[7] * D_801F1054[9];
  work = (s16 *)(tint - 5);
  shape = (SVECTOR *)&D_1F800014;
  mid = shape + 1;

  *(u16 *)0x1F800018 = 0;
  *(u16 *)0x1F800016 = 0;
  shape->vx = 0;

  i = 0;
  do {
    ((struct ScratchScalar *)work)->radius0 = D_801F041C[i];
    work[1] = D_801F0424[i];
    work[2] = D_801F042C[i];
    work[3] = D_801F0434[i];
    work[4] = ((D_801F1054[0xB] + D_801F043C[i]) & 0xF) << 8;
    prim = (POLY_G3 *)g_PrimCursor;

    func_8017A97C(prim);
    SetSemiTrans(prim, 1);

    mid->vx = (func_801782FC(work[2]) * work[0]) >> 12;
    mid->vy = (func_801783C8(work[2]) * work[0]) >> 12;
    (mid + 1)->vx = (func_801782FC(work[3]) * work[1]) >> 12;
    (mid + 1)->vy = (func_801783C8(work[3]) * work[1]) >> 12;
    z = (u32)func_801782FC(work[4]) >> 7;
    (mid + 1)->vz = mid->vz = z;

    RotTransPers3(mid - 1, mid, mid + 1, (long *)&prim->x0, (long *)&prim->x1,
                  (long *)&prim->x2, &depth, &flag);

    prim->r0 = ((u16 *)work)[7];
    prim->g0 = ((u16 *)work)[6];
    prim->b0 = ((u16 *)work)[5];
    prim->r1 = ((u16 *)work)[5];
    prim->g1 = ((u16 *)work)[6];
    prim->b1 = ((u16 *)work)[7];
    prim->r2 = ((u16 *)work)[5];
    prim->g2 = ((u16 *)work)[6];
    prim->b2 = ((u16 *)work)[7];

    func_80155A08(*(s32 *)(D_801F1054 + 0x10), *(s32 *)(D_801F1054 + 0x14), 2,
                  0x1C);
    i++;
  } while (i < 4);
}
