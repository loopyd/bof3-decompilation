#include "bof3/bof3.h"
#include "gpu/prim.h"

/* The overlay sprite-record emitter at 0x801F3A10, still an asm sibling in this
 * target: it selects its draw mode, queues one semi-transparent SPRT from its
 * own record table and sizes it from the entry its third argument indexes, so
 * this body keeps that argument as a plain immediate. Its two call sites here
 * take no result. */
void func_801F3A10(s16 arg0, s16 arg1, u8 arg2);

/* @source 0x8014832E @kind unknown */
extern u8 D_8014832E;

/* @source 0x80010004 @kind unknown */
extern u16 D_80010004;

/* @source 0x801F40FC
 * @behavior Draws the AREA065 sprite frame and its main-RAM texture row unless
 *           the shared display-enable byte 0x8014832E holds none of bits 0x1B.
 *           Through the overlay sprite-record emitter it queues entry 4 at
 *           (x, y) and entry 5 at (x + 0x80, y), then submits the texture
 *           whose handle is the 0x80010000 main-RAM base plus the offset-table
 *           halfword D_80010004, the third entry of that halfword table, at
 *           (x + 4, y + 4) with colour 0 and flag 0xFF through func_8014F800.
 *           Both arguments are narrowed with `sll`/`sra 16` in this body. The
 *           three state handlers that reach it pass x 0x5C and the animated
 *           work halfword 0x30 as y. Returns nothing. The 0xB4 bytes are
 *           byte-identical to the spans at 0x801F40C4 in emi/world00/area016/13
 *           and emi/world01/area045/13 apart from the low half of the two
 *           internal `jal` words, which carry each overlay's own emitter
 *           address, and to the span at 0x801F3FF8 in emi/world00/area033/13
 *           apart from those `jal` words and the halfword offset cell, so this
 *           body transcribes that family's statement order with this target's
 *           emitter and offset cell.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawSpriteFrameAndMainRamTexture2World01Area06513_801F40FC(s16 arg0,
                                                               s16 arg1) {
  s16 y;

  if ((D_8014832E & 0x1bu) != 0u) {
    y = arg1;
    func_801F3A10(arg0, y, 4u);
    func_801F3A10((s16)(arg0 + 0x80), y, 5u);
    func_8014F800((s16)(arg0 + 4), (s16)(arg1 + 4), 0, 0xffu,
                  0x80010000u + (u32)D_80010004);
  }
}
