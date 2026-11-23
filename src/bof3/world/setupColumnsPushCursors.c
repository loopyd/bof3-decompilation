#include "bof3/bof3.h"

/* The two layer push cursors, one byte per layer, holding the record slot each
 * layer writes its next sample into. */
/* @source 0x801F44B4 @kind bss */
extern u8 trailLayerPushCursors[];

/* The 0x20-byte-stride sample slots of the trail buffer, three bytes per layer
 * at +0, +1 and +2: the sample column, the sample row and the flip flag. */
/* @source 0x801F54D0 @kind data */
extern u8 D_801F54D0[];
/* @source 0x801F54D2 @kind data */
extern u8 D_801F54D2[];
/* @source 0x801F54D5 @kind data */
extern u8 D_801F54D5[];

/* Shared overlay state byte of the main executable, cleared here and set to 1 by
 * the only caller of this body. */
/* @source 0x8014933B @kind data */
extern u8 D_8014933B;

/* @source 0x801F2E14
 * @behavior Resets the two-layer trail of the overlay: it clears both entries of
 *           trailLayerPushCursors, then for each layer walks the thirty-two
 *           slots of the trail buffer and writes 0xFF into the slot's sample
 *           column byte, the slot index marching down from 0xBA by six bytes
 *           per step plus the layer's own three-byte offset. It then writes the
 *           flip flag of slot 0 as 1 on the first layer and 0 on the second and
 *           clears the shared overlay state byte at 0x8014933B. Walks the
 *           layers in order, allocates no frame, calls nothing, and has one exit
 *           through $ra.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setupColumnsPushCursors(
    void) {
  s32 layer;
  u8 fill;
  s32 offset;
  s32 i;
  s32 index;

  layer = 0;
  fill = 0xFF;
  offset = 0;
  do {
    trailLayerPushCursors[layer] = 0;
    i = 0x1F;
    index = offset + 0xBA;
    do {
      D_801F54D0[index] = fill;
      i -= 1;
      index -= 6;
    } while (i >= 0);
    layer += 1;
    offset += 3;
  } while (layer < 2);
  D_801F54D2[0] = 1;
  D_801F54D5[0] = 0;
  D_8014933B = 0;
}
