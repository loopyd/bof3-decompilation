#include "bof3/bof3.h"

/* Shared display-enable byte in main RAM, outside every overlay payload: the
 * exact sibling drawer at 0x801F40DC gates this same sprite frame on the same
 * bits. */
extern u8 D_8014832E; /* @source 0x8014832E @kind unknown */

/* The local emitter at 0x801F3A0C selects one four-byte entry of the local
 * sprite table 0x801F5770 with its third argument and emits the
 * semi-transparent sprite at the two signed coordinates it is given. */
void func_801F3A0C(s16 arg0, s16 arg1, u8 arg2);
void func_8014F800(s16 arg0, s16 arg1, s32 arg2, u32 arg3, u32 arg4);

/* @source 0x801F40F8
 * @behavior Draws the AREA087 sprite frame unless the shared display-enable
 *           byte 0x8014832E holds none of bits 0x1B: the 0x80-by-0x18
 *           sprite-table entry 4 of the local table 0x801F5770, whose texture
 *           cell is (0, 0x38), at (x, y) and the 8-by-0x18 end-cap entry 5 at
 *           texture (0, 0x50) at (x + 0x80, y) through the semi-transparent
 *           sprite emitter 0x801F3A0C, then the texture whose handle is the
 *           0x80010000 main-RAM base plus offset-table entry 0, at (x + 4,
 *           y + 4) with colour 0 and flag 0xFF through func_8014F800. The three
 *           state handlers 0x801F3784, 0x801F385C and 0x801F395C pass x 0x5C
 *           and the animated work halfword at record offset 0x30 as y. Takes
 *           two signed halfword coordinates and returns nothing. Its 0xB0
 *           original bytes matched the exact
 *           drawSpriteFrameAndMainRamTexture0World03Area12114_801F40DC lift in
 *           emi/world03/area121/14 in every word but the two internal `jal`
 *           words, each lower there by the 0x1C the two spans sit apart, so the
 *           two spans are one body and this source follows that sibling's
 *           statement order with this target's own emitter address.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawSpriteFrameAndMainRamTexture0World02Area08713_801F40F8(
    s16 arg0, s16 arg1) {
  s16 y;

  if ((D_8014832E & 0x1bu) != 0u) {
    y = arg1;
    func_801F3A0C(arg0, y, 4u);
    func_801F3A0C((s16)(arg0 + 0x80), y, 5u);
    func_8014F800((s16)(arg0 + 4), (s16)(arg1 + 4), 0, 0xffu,
                  0x80010000u + PSX_PTR(u16, 0x80010000u)[0]);
  }
}
