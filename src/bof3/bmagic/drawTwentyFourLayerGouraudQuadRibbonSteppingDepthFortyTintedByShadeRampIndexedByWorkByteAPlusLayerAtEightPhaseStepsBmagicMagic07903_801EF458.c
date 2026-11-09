#include "bof3/bof3.h"
#include "gpu/prim.h"

/* Scratchpad work-object pointer cell at 0x1F800044, bound by this target's map
 * as g_battle_work, the name every lifted body of this overlay already uses.
 * The original re-materialises the cell constant per access (lui 0x1F80 plus lw
 * 0x44) because the primitive stores in the loop may alias it. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared frame counter in main RAM. The low six bits of it seed the phase of
 * the ring sampler once before the loop, and its value plus the layer index
 * seeds the sampler once per layer. */
extern s32 D_80143E6C; /* @source 0x80143E6C @kind data */

/* The two sixteen-word shade ramps inside this overlay's trailing read-only
 * block D_801F02A4. The first runs 0x60, 0x80, 0xA0, 0xC0, 0xC0, 0xA0, 0x80,
 * 0x60, 0x40, 0x20, 0x10, 0x08, 0x08, 0x10, 0x20, 0x40 and the second 0x40,
 * 0x20, 0x10, 0x08, 0x08, 0x10, 0x20, 0x40, 0x40, 0x20, 0x10, 0x08, 0x08,
 * 0x10, 0x20, 0x40. Both are indexed by a four-bit shade index and their words
 * are truncated to the halfword the ring publishes. The block stays one Splat
 * `textbin` row in the target map, so these declarations only bind the two
 * sub-spans by their address labels. */
extern u32 D_801F02DC[16]; /* @source 0x801F02DC @kind table */
extern u32 D_801F031C[16]; /* @source 0x801F031C @kind table */

/* Scratchpad ring state at 0x1F800000, one record of eight halfwords. The
 * running angle and the angle of the previous layer are signed - the original
 * reads both back with `lh` for the four scale products. The near and far depth
 * halfwords, the near and far shade halfwords and the seam angle are unsigned;
 * the colour stores read the shade halfwords back one byte at a time and the
 * third and half channels re-read them through an explicit signed cast. The
 * four-bit shade index is signed, which is why the re-seed paths reload it with
 * `lh`. The original folds member offsets into the `%hi`/`%lo` pair of some
 * statements (the plain `lui $at` stores) and hoists constant bases at
 * 0x1F800006 for the rest, so this is one record object and not eight cells. */
typedef struct BmagicMagic07903RingCells {
  s16 angle;      /* 0x00 */
  s16 anglePrev;  /* 0x02 */
  u16 z;          /* 0x04 */
  u16 zPrev;      /* 0x06 */
  u16 shade;      /* 0x08 */
  u16 shadePrev;  /* 0x0A */
  s16 index;      /* 0x0C */
  u16 seam;       /* 0x0E */
} BmagicMagic07903RingCells;

extern BmagicMagic07903RingCells D_1F800000; /* @source 0x1F800000 @kind data */

/* Scratchpad vertex quad at 0x1F800014: the four SVECTOR inputs of
 * RotTransPers4. The ring copies the two trailing vertices into the two
 * leading ones field by field and then builds the new trailing pair in place.
 * The original addresses the quad as one eight-byte-stride object, forming the
 * base at its second vertex and deriving the first from it. */
extern SVECTOR D_1F800014[4]; /* @source 0x1F800014 @kind data */

/* Shared graphic-type query used for the draw-mode tpage argument. */
s32 GetGraphType(void);

/* Shared draw-mode packet writer; this body opens one 0xC-byte packet on the
 * primitive cursor and links it through func_80155A08. */
void SetDrawMode(DR_MODE *p, int dfe, int dtd, int tpage, RECT *tw);

/* Shared primitive setters: SetPolyG4 stamps the tag of the current
 * g_PrimCursor packet, RotTransPers4 projects the four scratchpad vertices
 * into its corner pairs and homes the depth and flag words in the frame. */
void SetPolyG4(POLY_G4 *p);
long RotTransPers4(SVECTOR *v0, SVECTOR *v1, SVECTOR *v2, SVECTOR *v3,
                   long *sxy0, long *sxy1, long *sxy2, long *sxy3, long *p,
                   long *flag);

/* Shared primitive linker: it resolves the record cell word pair at 0x34 and
 * 0x38 of the work object, links a primitive at g_PrimCursor with the given
 * kind and advances the cursor by the given size class, which is why this body
 * links one 0xC-byte packet before the rings and one 0x24-byte POLY_G4 packet
 * per quad. */
void func_80155A08(s32 arg0, s32 arg1, s32 arg2, s32 arg3);

/* Shared rotator pair at 0x801782FC and 0x801783C8: the sine and cosine samples
 * of a 12-bit phase in the fixed-point scale the `>> 12` products below undo. */
s32 func_801782FC(s32 arg0);
s32 func_801783C8(s32 arg0);

/* @source 0x801EF458
 * @behavior Ring emitter of this overlay's work-byte-2 handler group. Its only
 *           in-image references are the jal at 0x801EEE44 inside
 *           dispatchWorkByte2HandlerThenDrawByWorkByte4BmagicMagic07903 and the
 *           jal at 0x801EF110 inside func_801EF0A8; both call it between the
 *           matrix load func_801EF3A8 and PopMatrix, so it draws into the
 *           record's matrix.
 *           It opens the draw mode on the primitive cursor with tpage 0x25
 *           when GetGraphType returns 1 or 2 and 0x15 otherwise, links that
 *           0xC-byte packet, then seeds the ring. The seed clears the near
 *           depth at 0x1F800004, publishes work byte 0xB doubled as the seam
 *           angle at 0x1F80000E, samples the frame counter's low six bits
 *           through func_801782FC and keeps three twelfths of the sample as
 *           the near spread, publishes the low nibble of work byte 0xA as the
 *           shade index at 0x1F80000C, and sets the near shade and the running
 *           angle from the second ramp 0x801F031C when work byte 0x2 is below
 *           two. For every other work byte 0x2 it uses the first ramp
 *           0x801F02DC, adds the sampled turn scaled by four and shifted right
 *           twelve to the seam angle, and, when work bytes 0x1 and 0x9 are both
 *           set, re-seeds shade and angle from the second ramp again.
 *           Twenty-four layers follow. The near spread and the far spread are
 *           the first halfword of a two-vertex scratch vector in the frame, so
 *           the layer keeps the previous spread in memory rather than a
 *           register; the low six bits of the frame counter plus the layer
 *           seed the sampler once per layer, and the sample times three times
 *           the layer, shifted right twelve, becomes the new near spread. The
 *           layer moves the previous near depth, shade and running angle into
 *           their far cells 0x1F800006, 0x1F80000A and 0x1F800002, steps the
 *           near depth to minus forty times the layer, and publishes the shade
 *           index low nibble of work byte 0xA plus the layer.
 *           Work byte 0x2 then selects one of three shapes. Two and three seed
 *           the near shade from the first ramp and re-set the running angle
 *           from the seam angle plus the sampled turn, weighted by three times
 *           the layer plus four while the shade index is below eight and by the
 *           plain scaled turn above it; their ring-length test against work byte
 *           0x9 takes its direction from work byte 0x1, in opposite senses for
 *           the two values, and re-seeds the running angle from the seam angle
 *           and the near shade from the second ramp when the test holds. Every
 *           other value seeds the near shade from the second ramp through the
 *           signed shade index and re-sets the running angle from the seam angle
 *           plus the scaled turn.
 *           The layer then builds the trailing edge at phase zero from the
 *           near and far angles, spreads and depths and emits eight quads. Each
 *           step copies the trailing edge into the leading pair field by field,
 *           builds the new near edge at the step phase times the running angle
 *           and the new far edge at the same phase times the previous running
 *           angle, both scaled right twelve, and appends a 0x24-byte POLY_G4
 *           whose two leading corners take the near shade and whose two
 *           trailing corners take the far shade, the second and third channels
 *           being the shade third and half. Takes no arguments, returns nothing
 *           and reads no incoming register.
 * @status partial
 * @match 91.55
 * @residual The candidate stops 41 instructions and 32 bytes short of the
 *           original from +0x0090, where the trailing-edge build and its phase
 *           steps differ; the emitted layers before that point match.
 */
void drawTwentyFourLayerGouraudQuadRibbonSteppingDepthFortyTintedByShadeRampIndexedByWorkByteAPlusLayerAtEightPhaseStepsBmagicMagic07903_801EF458(
    void) {
  /* The two x spreads of the ring edges live in the frame as the first
   * halfword of a two-vertex scratch vector: the original keeps them in memory
   * across the inner loop (sh/lhu of two eight-byte slots) instead of a
   * register, which is what leaves the vertex and cell bases their saved
   * registers. */
  POLY_G4 *prim;
  long depth;
  long flag;
  s32 sample;
  s32 phase;
  s32 tpage;
  s32 layer;
  s32 step;
  s32 scale;
  u16 *seam;
  SVECTOR offsets[2];

  if (GetGraphType() == 1 || GetGraphType() == 2) {
    tpage = 0x25;
  } else {
    tpage = 0x15;
  }
  SetDrawMode((DR_MODE *)g_PrimCursor, 0, 1, tpage, 0);
  func_80155A08(*(s32 *)(g_battle_work + 0x34),
                *(s32 *)(g_battle_work + 0x38), 0, 0xC);

  seam = &D_1F800000.seam;
  D_1F800000.z = 0;
  *seam = g_battle_work[0xB] * 2;
  sample = func_801782FC((D_80143E6C & 0x3F) << 6);
  offsets[0].vx = (sample * 3) >> 12;
  D_1F800000.index = g_battle_work[0xA] & 0xF;
  if (g_battle_work[2] >= 2) {
    D_1F800000.shade = D_801F02DC[D_1F800000.index];
    D_1F800000.angle = *seam + ((func_801782FC(D_1F800000.index << 8) << 2) >> 12);
    if (g_battle_work[1] != 0 && g_battle_work[9] != 0) {
      D_1F800000.shade = D_801F031C[D_1F800000.index];
      D_1F800000.angle = *seam + ((func_801782FC(D_1F800000.index << 8) << 2) >> 12);
    }
  } else {
    D_1F800000.shade = D_801F031C[D_1F800000.index];
    D_1F800000.angle = *seam + ((func_801782FC(D_1F800000.index << 8) << 2) >> 12);
  }


  layer = 1;
  scale = 7;
  do {
    offsets[1].vx = offsets[0].vx;
    D_1F800000.zPrev = D_1F800000.z;
    D_1F800000.z = -(layer * 40);
    sample = func_801782FC(((D_80143E6C + layer) & 0x3F) << 6) * 3;
    offsets[0].vx = (sample * layer) >> 12;
    D_1F800000.shadePrev = D_1F800000.shade;
    D_1F800000.anglePrev = D_1F800000.angle;
    D_1F800000.index = (g_battle_work[0xA] + layer) & 0xF;
    if (g_battle_work[2] == 2) {
      D_1F800000.shade = D_801F02DC[D_1F800000.index];
      if (D_1F800000.index < 8) {
        D_1F800000.angle =
            D_1F800000.seam +
            ((func_801782FC(D_1F800000.index << 8) * scale) >> 12);
      } else {
        D_1F800000.angle =
            D_1F800000.seam +
            ((func_801782FC(D_1F800000.index << 8) << 2) >> 12);
      }
      if (g_battle_work[1] != 0) {
        if (layer < g_battle_work[9]) {
          D_1F800000.angle = D_1F800000.seam;
          D_1F800000.shade = D_801F031C[D_1F800000.index];
        }
      } else {
        if (g_battle_work[9] < layer) {
          D_1F800000.angle = D_1F800000.seam;
          D_1F800000.shade = D_801F031C[D_1F800000.index];
        }
      }
    } else if (g_battle_work[2] == 3) {
      D_1F800000.shade = D_801F02DC[D_1F800000.index];
      if (D_1F800000.index < 8) {
        D_1F800000.angle =
            D_1F800000.seam +
            ((func_801782FC(D_1F800000.index << 8) * scale) >> 12);
      } else {
        D_1F800000.angle =
            D_1F800000.seam +
            ((func_801782FC(D_1F800000.index << 8) << 2) >> 12);
      }
      if (g_battle_work[1] == 0) {
        if (layer < g_battle_work[9]) {
          D_1F800000.angle = D_1F800000.seam;
          D_1F800000.shade = D_801F031C[D_1F800000.index];
        }
      } else {
        if (g_battle_work[9] < layer) {
          D_1F800000.angle = D_1F800000.seam;
          D_1F800000.shade = D_801F031C[D_1F800000.index];
        }
      }
    } else {
      D_1F800000.shade = D_801F031C[D_1F800000.index];
      D_1F800000.angle =
          D_1F800000.seam + ((func_801782FC(D_1F800000.index << 8) << 2) >> 12);
    }

    D_1F800014[1].vx = offsets[0].vx + ((func_801783C8(0) * D_1F800000.angle) >> 12);
    D_1F800014[1].vy = (func_801782FC(0) * D_1F800000.angle) >> 12;
    D_1F800014[1].vz = D_1F800000.z;
    D_1F800014[3].vx = offsets[1].vx + ((func_801783C8(0) * D_1F800000.anglePrev) >> 12);
    D_1F800014[3].vy = (func_801782FC(0) * D_1F800000.anglePrev) >> 12;
    D_1F800014[3].vz = D_1F800000.zPrev;

    step = 1;
    do {
      D_1F800014[0].vx = D_1F800014[1].vx;
      D_1F800014[0].vy = D_1F800014[1].vy;
      D_1F800014[0].vz = D_1F800014[1].vz;
      D_1F800014[2].vx = D_1F800014[3].vx;
      D_1F800014[2].vy = D_1F800014[3].vy;
      D_1F800014[2].vz = D_1F800014[3].vz;

      phase = step << 9;
      D_1F800014[1].vx = offsets[0].vx + ((func_801783C8(phase) * D_1F800000.angle) >> 12);
      D_1F800014[1].vy = (func_801782FC(phase) * D_1F800000.angle) >> 12;
      D_1F800014[1].vz = D_1F800000.z;
      D_1F800014[3].vx = offsets[1].vx + ((func_801783C8(phase) * D_1F800000.anglePrev) >> 12);
      D_1F800014[3].vy = (func_801782FC(phase) * D_1F800000.anglePrev) >> 12;
      D_1F800014[3].vz = D_1F800000.zPrev;

      prim = (POLY_G4 *)g_PrimCursor;
      SetPolyG4(prim);
      prim->r0 = D_1F800000.shade;
      prim->g0 = (s16)D_1F800000.shade / 3;
      prim->b0 = (s16)D_1F800000.shade / 2;
      prim->r1 = D_1F800000.shade;
      prim->g1 = (s16)D_1F800000.shade / 3;
      prim->b1 = (s16)D_1F800000.shade / 2;
      prim->r2 = D_1F800000.shadePrev;
      prim->g2 = (s16)D_1F800000.shadePrev / 3;
      prim->b2 = (s16)D_1F800000.shadePrev / 2;
      prim->r3 = D_1F800000.shadePrev;
      prim->g3 = (s16)D_1F800000.shadePrev / 3;
      prim->b3 = (s16)D_1F800000.shadePrev / 2;

      RotTransPers4(D_1F800014, D_1F800014 + 1, D_1F800014 + 2,
                    D_1F800014 + 3,
                    (long *)&prim->x0, (long *)&prim->x1, (long *)&prim->x2,
                    (long *)&prim->x3, &depth, &flag);
      func_80155A08(*(s32 *)(g_battle_work + 0x34),
                    *(s32 *)(g_battle_work + 0x38), 0, 0x24);
      step++;
    } while (step < 9);

    layer++;
    scale += 3;
  } while (layer < 0x19);
}
