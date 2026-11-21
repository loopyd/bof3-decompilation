#include "bof3/bof3.h"

/* Shared overlay state byte of the main executable: the trail reset this body
 * calls clears it and this body arms it to 1. */
/* @source 0x8014933B @kind data */
extern u8 D_8014933B;

/* The trail reset this overlay entry runs before arming the state byte. */
extern void
initTwoLayerTrailSampleColumnsAndPushCursorsWorld04Area19913_801F2E14(void);

/* @source 0x801F2DE8
 * @behavior Overlay entry hook that starts the two-layer sample trail: it calls
 *           the trail reset at 0x801F2E14, which clears both push cursors and
 *           every slot sample column and clears the shared overlay state byte at
 *           0x8014933B, then stores 1 into that byte to mark the trail as
 *           running. Takes no arguments, returns no value, allocates the
 *           0x18-byte frame the call requires, saving $ra at 0x10($sp), and has
 *           one exit through $ra.
 * @status exact
 * @match 100.00
 * @residual none
 */
void startTwoLayerTrailAndSetOverlayStateByteWorld04Area19913_801F2DE8(void) {
  initTwoLayerTrailSampleColumnsAndPushCursorsWorld04Area19913_801F2E14();
  D_8014933B = 1;
}
