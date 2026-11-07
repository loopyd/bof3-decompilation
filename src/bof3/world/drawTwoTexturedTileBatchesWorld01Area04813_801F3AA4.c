#include "bof3/bof3.h"

/* Unlifted eight-quad tile emitter in this overlay: its own bytes take a
 * halfword position, a texture-page value and an index byte, project eight
 * textured quads and submit them through the shared append helper.
 */
void func_801F3AE4(s16 position, u32 texturePage, s16 index);

/* @source 0x801F3AA4
 * @behavior Emits two batches of eight textured tiles by invoking the tile
 * emitter at 0x801F3AE4 twice: position 0x400 with texture page 0x848000 and
 * index 0x10, then position 0xC00 with texture page 0x888000 and index 0x11.
 * Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawTwoTexturedTileBatchesWorld01Area04813_801F3AA4(void) {
  func_801F3AE4(0x400, 0x848000, 0x10);
  func_801F3AE4(0xC00, 0x888000, 0x11);
}
