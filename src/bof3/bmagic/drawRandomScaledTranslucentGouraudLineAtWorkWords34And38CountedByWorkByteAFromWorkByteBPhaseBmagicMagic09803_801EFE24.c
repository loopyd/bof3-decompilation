#include "bof3/bof3.h"
#include "gpu/prim.h"
#include <stdlib.h>

/* Scratchpad work-object pointer cell 0x1F800044, bound by this target's map.
 * The original re-materialises the cell with its own lui/lw pair at each of the
 * three groups that read the record - the phase seed, each pass's phase and
 * each pass's append - so the plain named cell suffices. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Scratchpad pass block this step seeds and then addresses through one halfword
 * pointer biased at 0x1F800002: the radial scale the sampler's result is
 * multiplied by at -0x02, the phase handed to the sampler at 0x00, the random
 * shade both vertex colours are taken from at +0x02 and the pass count at
 * +0x06. The scale seed goes through a second pointer held on 0x1F800000
 * itself, which is the base the loop's pointer is stepped off. The seed stores
 * reach the block by its literal cells; only the loop's repeated accesses go
 * through the pointer. */
extern s16 D_1F800000; /* @source 0x1F800000 @kind data */
extern s16 D_1F800002; /* @source 0x1F800002 @kind data */
extern u16 D_1F800004; /* @source 0x1F800004 @kind data */
extern s16 D_1F800008; /* @source 0x1F800008 @kind data */

/* The one projected vertex both projections of a pass share: x is cleared per
 * vertex, y is the sampled radial offset the pass's scale produces and z is the
 * negated index shifted left six. */
extern SVECTOR D_1F800014; /* @source 0x1F800014 @kind data */

/* Resident helpers, all in the shared main executable rather than this
 * overlay: the graphic-type query the draw-mode tpage follows, the radial
 * sampler the phase is handed to, the packet stamp, and the ordered-list
 * append both the 0x0C-byte draw-mode packet and the 0x14-byte line are queued
 * through from the work record's screen word pair 0x34/0x38. */
s32 GetGraphType(void);
s32 func_801782FC(s32 arg0);
void func_8017AA94(void *arg0);
void func_80155A08(s32 arg0, s32 arg1, s32 arg2, s32 arg3);

/* @source 0x801EFE24
 * @behavior The argument-free translucent line step of this overlay's magic
 *           effect. It selects the draw-mode tpage from GetGraphType - 0xA5 for
 *           graphic type 1 and for graphic type 2, 0x35 otherwise - queues that
 *           0x0C-byte draw-mode packet on the shared primitive cursor
 *           0x8014598C under OT slot 2 from the work record's screen words 0x34
 *           and 0x38, and seeds its own scratchpad pass block: 0x80 as the
 *           radial scale at 0x1F800000, the record's byte 0xB plus two masked
 *           to its low nibble and shifted up eight bits as the phase at
 *           0x1F800002, the record's byte 0xA as the pass count at 0x1F800008
 *           and rand() & 0x3F as the shade at 0x1F800004. It then samples the
 *           shared radial sampler 0x801782FC at the seeded phase, keeps
 *           (sample * scale) >> 12 as the projected vertex's y at 0x1F800016
 *           and zeroes that vertex's x and z.
 *
 *           Each pass re-reads the cursor as the Gouraud line packet, stamps it
 *           through 0x8017AA94, marks it translucent and projects the vertex
 *           into its x0/y0 pair at +0x08. It then re-randomises the radial
 *           scale to 0xA0 plus 0..0x1F or 0xA0 minus 0..0x1F, re-derives the
 *           phase from the record's byte 0xB plus the index plus two masked to
 *           its low nibble and shifted up eight bits, keeps that phase at
 *           0x1F800002, zeroes the vertex's x, samples the new phase, writes
 *           (sample * scale) >> 12 to the vertex's y and the negated index
 *           shifted left six to its z, and projects it into the packet's x1/y1
 *           pair at +0x10. Both endpoints take the shade halfword in red and
 *           green with 0x20 in blue; the shade is re-randomised to
 *           (rand() + 0x40) & 0x3F between the two endpoints. It closes by
 *           queuing the 0x14-byte line under OT slot 2 from the same work words
 *           0x34 and 0x38 and repeating while the index is below the pass count
 *           it re-reads from the block. Takes no arguments, returns nothing,
 *           keeps a 0x38-byte frame with $ra and five saved registers because
 *           of its calls, and reads and writes no other state.
 * @status partial
 * @match 97.62
 * @residual The loop-entry seed's `(sample * scale) >> 12` store precedes
 *           the entry test's count load and `slt`; the original keeps the
 *           result in $v1 and stores it after that comparison. The draw-mode
 *           selection and the remaining body match. Overall, 164 of 168
 *           instructions match at the original size of 672 bytes.
 */
/* Representation evidence for the two seed stores and the pass phase. The pass
 * phase is a signed halfword local: with an s32 local the two uses below
 * collapsed onto a single `sll`, while the halfword local and the `* 0x100`
 * argument spell the shift twice, which is what the original emits - the mask
 * once, then `sll` into $a0 for the sampler and `sll` into $v0 for the pass
 * block store. The scale seed is written through the pointer biased on
 * 0x1F800000 and read back through it, because the original reaches that store
 * and that load with its register; the pass loop's re-randomised scale store
 * instead names the cell, because the original reaches it by its literal
 * address there. */
void drawRandomScaledTranslucentGouraudLineAtWorkWords34And38CountedByWorkByteAFromWorkByteBPhaseBmagicMagic09803_801EFE24(
    void) {
  LINE_G2 *prim;
  SVECTOR *vertex;
  s16 *base;
  s16 *stage;
  s32 i;
  s32 graph;
  s16 phase;
  s32 sample;
  long projected;
  long depth;

  if (GetGraphType() == 1 || GetGraphType() == 2) {
    graph = 0xA5;
  } else {
    graph = 0x35;
  }

  SetDrawMode((DR_MODE *)g_PrimCursor, 0, 1, graph, 0);
  func_80155A08(FIELD_REF(s32, g_battle_work, 0x34u),
                FIELD_REF(s32, g_battle_work, 0x38u), 2, 0xC);

  base = &D_1F800000;
  *base = 0x80;
  D_1F800002 = (((((u8 *)g_battle_work)[0x0B]) + 2) & 0x0F) << 8;
  D_1F800008 = ((u8 *)g_battle_work)[0x0A];
  i = 1;
  D_1F800004 = rand() & 0x3F;
  vertex = &D_1F800014;
  vertex->vx = 0;
  sample = func_801782FC(D_1F800002);
  SPAD_REF(s16, 0x18u) = 0;
  SPAD_REF(s16, 0x16u) = (s16)((sample * *base) >> 12);
  stage = base + 1;

  while (i < stage[3]) {
    prim = (LINE_G2 *)g_PrimCursor;
    func_8017AA94(prim);
    SetSemiTrans(prim, 1);
    RotTransPers(&D_1F800014, (long *)&prim->x0, &projected, &depth);

    if (rand() & 1) {
      D_1F800000 = (rand() & 0x1F) + 0xA0;
    } else {
      D_1F800000 = 0xA0 - (rand() & 0x1F);
    }

    phase = (((((u8 *)g_battle_work)[0x0B]) + i + 2) & 0x0F);
    vertex->vx = 0;
    stage[0] = phase << 8;
    sample = func_801782FC(phase * 0x100);
    vertex->vy = (s16)((sample * stage[-1]) >> 12);
    vertex->vz = (s16)(-(i << 6));
    RotTransPers(vertex, (long *)&prim->x1, &projected, &depth);

    prim->r0 = *(u16 *)&stage[1];
    prim->g0 = *(u16 *)&stage[1];
    prim->b0 = 0x20;
    *(u16 *)&stage[1] = (rand() + 0x40) & 0x3F;
    prim->r1 = *(u16 *)&stage[1];
    prim->g1 = *(u16 *)&stage[1];
    prim->b1 = 0x20;

    func_80155A08(FIELD_REF(s32, g_battle_work, 0x34u),
                  FIELD_REF(s32, g_battle_work, 0x38u), 2, 0x14);
    i++;
  }
}
