#include "bof3/bof3.h"

extern u8 D_80145E98;
extern u8 D_80146865;

/* @source 0x801F2DBC
 * @behavior Overlay copier: publishes the leading mode byte of the current
 * game-mode record at 0x80145E98 into the area status byte at 0x80146865.
 * @status exact
 * @match 100.00
 * @residual none
 */
void copyModeByteWorld03Area13313_801F2DBC(void) {
  D_80146865 = D_80145E98;
}
