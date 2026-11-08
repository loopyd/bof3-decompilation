#include "bof3/bof3.h"
#include "gpu/prim.h"

#include <stdlib.h>

/* Active work record of this overlay, published through the scratchpad pointer
 * cell 0x1F800044. The cell is a variable rather than a constant address so that
 * each block reloads it, as the original does before the entry gate, before the
 * scroll/clamp block and before the projected quad pair.
 * @source 0x1F800044 @kind unknown
 */
extern u8* area12114WorkCursor_1F800044;

/* Shared effect bank byte at 0x8014832E: bit 0x04 lets this panel draw at all.
 * @source 0x8014832E @kind unknown
 */
extern u8 D_8014832E;

/* Shared frame counter at 0x80143E6C. Its low nibble is the per-frame scroll
 * step of the entry block and its low five bits are the phase of the second
 * quad's size pulse.
 * @source 0x80143E6C @kind unknown */
extern u32 D_80143E6C;

/* Shared map row byte at 0x80104001, the vertical bound the entry clamp tests.
 * @source 0x80104001 @kind unknown */
extern u8 D_80104001;

/* Shared overlay-focus halfwords at 0x8014930A/0x8014930E, the point the work
 * record halfwords 0x36/0x3A are compared against.
 * @source 0x8014930A @kind unknown */
extern s16 D_8014930A;
/* @source 0x8014930E @kind unknown */
extern s16 D_8014930E;

/* Four scratchpad vectors at 0x1F800014 that RotTransPers4 projects; the
 * original keeps their base in a saved register and writes the halfword lanes
 * through fixed addresses.
 * @source 0x1F800014 @kind data */
extern SVECTOR D_1F800014[];

/* Local helper at 0x80155560: stamps the texture window of the primitive from
 * its command word and layer index. */
/* @source 0x80155560 */
extern void func_80155560(u32 arg0, void* arg1, s32 arg2);

/* @source 0x801F477C
 * @behavior Two-layer scrolling textured panel of the work record published at
 * the scratchpad pointer cell 0x1F800044. The entry block runs once: while work
 * byte 0x02 is still zero it adds the low four bits of the shared frame counter
 * 0x80143E6C to the work halfword 0x3A and then stores the advanced byte, so the
 * two reads of byte 0x02 and the store order are that block's own.
 * The body returns unless bit 0x04 of the shared effect bank byte 0x8014832E is
 * set. It then derives the layer as work byte 0x0B minus 2, advances the scroll
 * word 0x38 by 0x800 plus the layer shifted left ten, and clamps the work
 * halfword 0x3A to -8 once the shared map byte 0x80104001 plus 8 no longer
 * exceeds it. A second gate follows on the reloaded record: the body returns
 * when both |0x8014930A - halfword 0x36| and |0x8014930E - halfword 0x3A| are
 * at least 0x1A, so it draws only near the shared overlay focus point.
 * It projects two POLY_FT4 quads over the scratchpad vectors at 0x1F800014,
 * both centred on the work words 0x34/0x38 shifted right nine and biased by
 * -0x3FC0, with a halfsize of (2 - layer) shifted left eight. The first quad
 * sits on z 0 and the second one repeats the same screen footprint on
 * z -0x300 with the halfsize grown by |0x0F - (frame counter & 0x1F)|, so the
 * two layers pulse apart. Each quad is stamped with SetPolyFT4 and SetShadeTex
 * with a zero argument, projected through RotTransPers4 into the primitive's
 * four sxy slots, has its texture window written by func_80155560 with the layer
 * in the low bits (0xBB28A100 for the z-0 layer, 0xBB509100 for the z-0x300
 * layer) and is appended by func_8014E5A0 at 0x28 bytes, to ordering table slot
 * 5 for the far layer and slot 4 for the near one. Takes no arguments and
 * returns nothing. Its 0x58-byte frame holds the set-up RotTransPers4 arguments
 * and the two long projection results at +0x10 through +0x2F and keeps
 * $ra,$fp,$s0-$s7 across the six calls.
 * @status partial
 * @match 84.21
 * @residual Live comparison: 192/228 instructions, 912 bytes against the
 * 900-byte original. The first difference at +0x0078 is a branch target shifted
 * by the extra twelve bytes. The scroll/clamp block uses the original's $a0,
 * but loads the scroll word before the layer byte rather than after it.
 * Three first-quad x-lane stores use absolute addresses rather than the saved
 * vector base. The x and vector-base saved registers are exchanged, and the
 * first quad's y stores are grouped rather than interleaved with x/z stores.
 * The frame, saved-register set and projection argument blocks agree.
 *
 * Shape evidence: the entry-gate, scroll, clamp and focus-test sequence is the
 * same statement order as the near-identical sibling func_801F4768 of the
 * area01613 overlay, whose entry block is recorded exact, and the quad
 * projection and texture-window calls follow the exact siblings
 * emitMeterBarWithPaletteWorld03Area12114_801F595C (0x801F595C) and
 * drawBlueToWhiteGradientBandFromScreenTopSizedByOverlayStateWorld03Area12114_801F6390
 * (0x801F6390) of this target. This function's own bytes fix the two command
 * words 0xBB28A100 and 0xBB509100, the ordering table slots 5 and 4, the two
 * depth literals 0 and -0x300, the -8 clamp, the 0x1A focus radius and the
 * 0x0F/-0x1F pulse. The y and z lanes are addressed through the vector symbol
 * and the x lanes through the saved vector base, which is the split the
 * original shows as its own `lui`/`at` pairs against one base register. The
 * span has no byte-identical twin anywhere under out/binaries, measured with
 * each `jal` immediate word masked, so no clone applies.
 */
void drawTwoLayerTexturedPanelNearSharedFocusWorld03Area12114_801F477C(void) {
  u8*       work;
  POLY_FT4* prim;
  SVECTOR*  vertices;
  s32       layer;
  u32       scroll;
  s32       half;
  s32       x;
  s32       y;
  long      depth;
  long      flag;

  work = area12114WorkCursor_1F800044;
  if (FIELD_REF(volatile u8, work, 0x02u) == 0) {
    u8 count;
    u8 next;

    count = FIELD_REF(volatile u8, work, 0x02u);
    next = count + 1;
    FIELD_REF(u16, work, 0x3au) += D_80143E6C & 0xfu;
    FIELD_REF(volatile u8, work, 0x02u) = next;
  }
  if ((D_8014832E & 4) == 0) {
    return;
  }

  {
    u8* scrollWork;

    scrollWork = area12114WorkCursor_1F800044;
    layer = (s32)FIELD_REF(u8, scrollWork, 0x0bu) - 2;
    scroll = FIELD_REF(u32, scrollWork, 0x38u);
    scroll = scroll + 0x800;
    scroll = scroll + (layer << 10);
    FIELD_REF(u32, scrollWork, 0x38u) = scroll;
    if (D_80104001 + 8 < FIELD_REF(s16, scrollWork, 0x3au)) {
      FIELD_REF(s16, scrollWork, 0x3au) = -8;
    }
  }

  work = area12114WorkCursor_1F800044;
  if (abs(D_8014930A - FIELD_REF(s16, work, 0x36u)) >= 0x1a &&
      abs(D_8014930E - FIELD_REF(s16, work, 0x3au)) >= 0x1a) {
    return;
  }

  prim = (POLY_FT4*)g_PrimCursor;
  y = (FIELD_REF(s32, work, 0x38u) >> 9) - 0x3fc0;
  x = (FIELD_REF(s32, work, 0x34u) >> 9) - 0x3fc0;
  SetPolyFT4(prim);
  SetShadeTex(prim, 0);

  vertices = D_1F800014;
  half = (2 - layer) << 8;
  vertices[0].vx = x - half;
  D_1F800014[0].vy = y - half;
  D_1F800014[0].vz = 0;
  vertices[1].vx = x + half;
  D_1F800014[1].vy = y - half;
  D_1F800014[1].vz = 0;
  vertices[2].vx = x - half;
  D_1F800014[2].vy = y + half;
  D_1F800014[2].vz = 0;
  vertices[3].vx = x + half;
  D_1F800014[3].vy = y + half;
  D_1F800014[3].vz = 0;

  RotTransPers4(vertices, vertices + 1, vertices + 2, vertices + 3,
                (long*)((u8*)prim + 8), (long*)((u8*)prim + 0x10),
                (long*)((u8*)prim + 0x18), (long*)((u8*)prim + 0x20), &depth,
                &flag);
  func_80155560(layer | 0xbb28a100u, (void*)prim, 1);
  GpuAppendPrim(5, 0x28);

  prim = (POLY_FT4*)g_PrimCursor;
  SetPolyFT4(prim);
  SetShadeTex(prim, 0);

  D_1F800014[0].vz = -0x300;
  D_1F800014[1].vz = -0x300;
  D_1F800014[2].vz = -0x300;
  D_1F800014[3].vz = -0x300;
  half += abs(0xf - (D_80143E6C & 0x1fu));
  vertices[0].vx = x - half;
  D_1F800014[0].vy = y - half;
  vertices[1].vx = x + half;
  D_1F800014[1].vy = y - half;
  vertices[2].vx = x - half;
  D_1F800014[2].vy = y + half;
  vertices[3].vx = x + half;
  D_1F800014[3].vy = y + half;

  RotTransPers4(vertices, vertices + 1, vertices + 2, vertices + 3,
                (long*)((u8*)prim + 8), (long*)((u8*)prim + 0x10),
                (long*)((u8*)prim + 0x18), (long*)((u8*)prim + 0x20), &depth,
                &flag);
  func_80155560(layer | 0xbb509100u, (void*)prim, 1);
  GpuAppendPrim(4, 0x28);
}
