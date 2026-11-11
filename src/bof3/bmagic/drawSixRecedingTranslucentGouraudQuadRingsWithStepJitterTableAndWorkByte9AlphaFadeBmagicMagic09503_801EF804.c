#include "bof3/bof3.h"
#include "gpu/prim.h"

/* Shared battle work pointer in the scratchpad, bound by this target's map.
 * The reads fold into lui plus the 0x44 displacement, the form the original
 * emits. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Scratchpad base cell at 0x1F800004. The original keeps one pointer to it in a
 * callee-saved register for the whole body and reaches the sampler amplitude at
 * 0x1F800000 as that pointer's word below, so both cells are addressed through
 * this one binding. */
extern s32 D_1F800004; /* @source 0x1F800004 @kind data */

/* Scratchpad fade cell at 0x1F800008. The primitive byte stores that follow the
 * publish go through the cursor pointer, so every read of the cell after them
 * reloads it, the form the original emits. */
extern s32 D_1F800008; /* @source 0x1F800008 @kind data */

/* Scratchpad vertex array at 0x1F800014: four SVECTORs the body samples into
 * and projects from. The third vertex at 0x1F800024 is that array plus two
 * vectors, and the inner loop addresses all four through that third-vertex
 * pointer, so the body binds the array once and walks forward. */
extern SVECTOR D_1F800014; /* @source 0x1F800014 @kind data */

/* Per-step radius and depth jitter table: one row of eight signed bytes per
 * ring, rows 0..5 for the inner edge and rows 1..6 for the outer edge. The
 * original reads a cell twice, once as a signed byte for the amplitude sum and
 * once as an unsigned byte sign-extended for the depth sum, so the table stays
 * a flat unsigned byte array with an explicit cast at the signed sites. */
extern u8 D_801F065C[]; /* @source 0x801F065C @kind table */

extern s32 GetGraphType(void);
extern s32 func_801782FC(s32 phase); /* @source 0x801782FC */
extern s32 func_801783C8(s32 phase); /* @source 0x801783C8 */
extern void func_80155A08(s32 x, s32 y, s32 kind, s32 size); /* @source 0x80155A08 */

/* @source 0x801EF804
 * @behavior Draws six nested rings of eight semi-transparent Gouraud quads.
 *           Each ring is one band between two radius rows: the inner edge row L
 *           and the outer edge row L+1. The band amplitude falls by 0x29 per
 *           ring from 0x100 and its published depth falls by 0x48 per ring from
 *           zero, so the rings recede and narrow toward the axis. Ring 6
 *           collapses: on the last band the outer edge vertices lose their
 *           lateral components, closing the disc to a point.
 *           Per-ring state is carried in stack words the original spills: the
 *           inner amplitude cell 0x1F800004 (0x100 minus 0x29 per ring), the
 *           inner edge pointer into the jitter table (plus eight bytes per
 *           ring), the ring counter one-based (plus one per ring), the inner
 *           depth offset (plus 0x48 per ring), the inner edge table offset
 *           (plus eight per ring), the outer depth base (plus 0x48 per ring),
 *           the outer amplitude term (plus 0x29 per ring) and the jitter table
 *           base (plus eight per ring). The ring body also copies that table
 *           offset and the outer edge pointer into the step loop's own pair of
 *           words once per ring, and the step loop and the fade read those
 *           copies while the ring tail advances the originals.
 *           Before the quads of a ring, the body samples phase zero into the
 *           first two vertices: the amplitude cell 0x1F800000 takes the ring's
 *           own amplitude plus that ring's table byte, and the sampler product
 *           shifted right twelve lands in vx and vy; 0x1F800004 takes the same
 *           amplitude and each vertex z takes the negated sum of the table byte
 *           and that edge's depth base. Eight steps then follow: each pass
 *           copies the first two vertices into the third and fourth slots,
 *           samples the new phase back into the first two, and draws the quad
 *           between that previous ring position and the new one, so eight
 *           passes close a full turn at 0x200 per step.
 *           The quad is projected from the vertex array through RotTransPers4
 *           into the four cursor pairs of a 0x24-byte POLY_G4, which is marked
 *           semi-transparent after a fresh cursor primitive is taken. Each pass
 *           first links a 0xC-byte draw-mode packet through 0x80155A08 at the
 *           screen position the third vertex projects to with work words 0x34
 *           and 0x38, and selects that packet's mode from GetGraphType, calling
 *           it separately per test: 0xA5 for graph type 1 or 2, otherwise 0x35.
 *           The tint is white with the blue channel carrying the fade: the
 *           band's fade is work byte 9 minus eight per ring, published to
 *           0x1F800008 as that value, as its 0x30 multiple when the value is
 *           below five, or as the 0x30 multiple of the value less four
 *           otherwise, capped at 0xC0 in both cases. Bands with a fade of four
 *           or less carry it on the first and third vertices and the value one
 *           on the second and fourth; the deeper bands carry it on all four.
 *           A band whose fade is not positive draws no quad at all: the step
 *           skips both the fade and the 0x24-byte link.
 *           The step handlers of this overlay at 0x801EEE60, 0x801EEF64,
 *           0x801EEFFC and 0x801EF070 call this with no arguments and consume no
 *           return value, so it draws with the matrix those handlers publish.
 *           What work byte 9 and the jitter table bytes mean beyond these
 *           published stores and sums is not established, so those cells stay
 *           offset-anchored. No payload under out/binaries holds this 0x5D8-byte
 *           span, so the statement order, operand forms and scratchpad access
 *           forms come from the original bytes rather than from a sibling body.
 * @status partial
 * @match 60.48
 * @residual 228/377 instructions at the original 0xB0-byte frame; the
 *           retained body is three instructions longer. Ring state and vertex
 *           pointers still occupy different stack words and saved registers.
 *           The step loop's band multiply becomes a strength-reduced stack
 *           term. Computing the wrapped step index at the loop header restores
 *           the original's leading andi and several inner-loop register roles.
 */
void drawSixRecedingTranslucentGouraudQuadRingsWithStepJitterTableAndWorkByte9AlphaFadeBmagicMagic09503_801EF804(
    void) {
  SVECTOR *shape;
  SVECTOR *next;
  POLY_G4 *prim;
  u8 *tbl;
  u8 *row;
  u8 *edge;
  s32 *base;
  s32 *fade;
  long depth;
  long flag;
  s32 graph;
  s32 a;
  s32 layer;
  s32 phase;
  s32 x;
  s32 y;
  s32 i;
  s32 j;
  s32 ampBase;
  s32 ampStep;
  s32 zoff;
  s32 zstep;
  s32 rowoff;
  s32 off;
  s32 n;
  s32 one;

  base = &D_1F800004;
  shape = &D_1F800014;
  next = shape + 2;
  one = 1;

  ampBase = 0x100;
  row = D_801F065C + 8;
  n = 1;
  zstep = 0x48;
  layer = 0;
  zoff = 0;
  rowoff = 0;
  ampStep = 0x29;
  tbl = D_801F065C;

  do {
    base[0] = ampBase;
    base[-1] = ampBase + *(s8 *)tbl;
    shape[0].vx = (func_801782FC(0) * base[-1]) >> 12;
    shape[0].vy = (func_801783C8(0) * base[-1]) >> 12;
    base[0] = 0x100 - ampStep;
    shape[0].vz = -((s8)tbl[0] + zoff);
    base[-1] = 0x100 - ampStep + *(s8 *)row;
    shape[1].vx = (func_801782FC(0) * base[-1]) >> 12;
    shape[1].vy = (func_801783C8(0) * base[-1]) >> 12;
    shape[1].vz = -((s8)row[0] + zstep);
    if (layer == 5) {
      shape[1].vx = shape[1].vy = 0;
    }
    off = rowoff;
    edge = row;
    i = 1;
    do {
      j = i & 7;
      next[0].vx = next[-2].vx;
      next[0].vy = next[-2].vy;
      next[0].vz = next[-2].vz;
      next[1].vx = next[-1].vx;
      next[1].vy = next[-1].vy;
      next[1].vz = next[-1].vz;
      phase = i << 9;
      base[0] = ampBase;
      base[-1] = ampBase + *(s8 *)&D_801F065C[off + j];
      next[-2].vx = (func_801782FC(phase) * base[-1]) >> 12;
      next[-2].vy = (func_801783C8(phase) * base[-1]) >> 12;
      a = 0x100 - n * 41;
      base[0] = a;
      next[-2].vz = -((s8)D_801F065C[off + j] + zoff);
      base[-1] = a + *(s8 *)&edge[j];
      next[-1].vx = (func_801782FC(phase) * base[-1]) >> 12;
      next[-1].vy = (func_801783C8(phase) * base[-1]) >> 12;
      next[-1].vz = -((s8)edge[j] + n * 72);
      if (layer == 5) {
        next[-1].vx = next[-1].vy = 0;
      }
      x = (next[-2].vx << 9) + *(s32 *)(g_battle_work + 0x34);
      y = (next[-2].vy << 9) + *(s32 *)(g_battle_work + 0x38);
      if (GetGraphType() == one || GetGraphType() == 2) {
        graph = 0xA5;
      } else {
        graph = 0x35;
      }
      SetDrawMode((DR_MODE *)g_PrimCursor, 0, 1, graph, 0);
      func_80155A08(x, y, 0, 0xC);
      prim = (POLY_G4 *)g_PrimCursor;
      SetPolyG4(prim);
      SetSemiTrans(prim, 1);
      RotTransPers4(shape, shape + 1, shape + 2, shape + 3, (long *)&prim->x0,
                    (long *)&prim->x1, (long *)&prim->x2, (long *)&prim->x3,
                    &depth, &flag);
      a = g_battle_work[9] - off;
      if (a > 0) {
        fade = &D_1F800008;
        *fade = a;
        if (a < 5) {
          *fade = a * 0x30;
          if (*fade > 0xC0) {
            *fade = 0xC0;
          }
          prim->r0 = one;
          prim->g0 = one;
          prim->b0 = *fade;
          prim->r1 = one;
          prim->g1 = one;
          prim->b1 = one;
          prim->r2 = one;
          prim->g2 = one;
          prim->b2 = *fade;
          prim->r3 = one;
          prim->g3 = one;
          prim->b3 = one;
        } else {
          a = (a - 4) * 0x30;
          *fade = a;
          if (*fade > 0xC0) {
            *fade = 0xC0;
            a = 0xC0;
          }
          prim->r0 = one;
          prim->g0 = one;
          prim->b0 = a;
          prim->r1 = one;
          prim->g1 = one;
          prim->b1 = *fade;
          prim->r2 = one;
          prim->g2 = one;
          prim->b2 = a;
          prim->r3 = one;
          prim->g3 = one;
          prim->b3 = *fade;
        }
        func_80155A08(x, y, 0, 0x24);
      }
      i++;
    } while (i < 9);
    zoff += 0x48;
    ampBase -= 0x29;
    row += 8;
    n += 1;
    rowoff += 8;
    zstep += 0x48;
    ampStep += 0x29;
    tbl += 8;
    layer++;
  } while (layer < 6);
}
