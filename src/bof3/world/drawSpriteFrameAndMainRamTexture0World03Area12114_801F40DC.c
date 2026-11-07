#include "bof3/bof3.h"

/* @source 0x8014832E @kind unknown */
extern u8 D_8014832E;

void func_801F39F0(s16 arg0, s16 arg1, u8 arg2);
void func_8014F800(s16 arg0, s16 arg1, s32 arg2, u32 arg3, u32 arg4);

/* @source 0x801F40DC
 * @behavior Draws the AREA121 sprite frame unless the shared display-enable
 *           byte 0x8014832E holds none of bits 0x1B: the 0x80-by-0x18
 *           sprite-table entry 4 at (x, y) and the 8-by-0x18 end-cap entry 5
 *           at (x + 0x80, y) through the semi-transparent sprite emitter
 *           0x801F39F0, then the texture whose handle is the 0x80010000
 *           main-RAM base plus offset-table entry 0, at (x + 4, y + 4) with
 *           colour 0 and flag 0xFF through func_8014F800. The three state
 *           handlers that reach it pass x 0x5C and the animated work halfword
 *           0x30 as y. Returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawSpriteFrameAndMainRamTexture0World03Area12114_801F40DC(
    s16 arg0, s16 arg1) {
  s16 y;

  if ((D_8014832E & 0x1bu) != 0u) {
    y = arg1;
    func_801F39F0(arg0, y, 4u);
    func_801F39F0((s16)(arg0 + 0x80), y, 5u);
    func_8014F800((s16)(arg0 + 4), (s16)(arg1 + 4), 0, 0xffu,
                  0x80010000u + PSX_PTR(u16, 0x80010000u)[0]);
  }
}
