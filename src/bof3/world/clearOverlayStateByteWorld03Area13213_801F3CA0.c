#include "bof3/bof3.h"

extern u8 D_8014933B;

/* @source 0x801F3CA0
 * @behavior Overlay state handler: clears the shared overlay state byte at
 *           0x8014933B to zero; takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearOverlayStateByteWorld03Area13213_801F3CA0(void) {
  D_8014933B = 0;
}
