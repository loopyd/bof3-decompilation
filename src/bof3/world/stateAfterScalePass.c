#include "bof3/bof3.h"

/* Palette block scaler of this overlay: 0x80 is the unity scale. */
void scalePaletteBlocks(s32 scale);

/* @source 0x801F2C48
 * @behavior Overlay step handler, entry 0 of the work-byte-2 handler table at
 *           0x801F3DFC: runs the payload palette block scaler with the zero
 *           scale, which fades both 0x100-entry palette blocks to black, then
 *           advances the dispatch step by incrementing byte 0x02 of the work
 *           record published at the scratchpad pointer cell 0x1F800044, the
 *           byte the exact dispatcher of this target reads as its table index.
 *           It takes no arguments and returns nothing; its 0x18-byte frame
 *           holds only the return address of the scaler call, and the frame
 *           restore is scheduled ahead of the record store.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateAfterScalePass(void) {
  scalePaletteBlocks(0);
  SPAD_PTR_SLOT(u8, 0x44)[2]++;
}
