#include "bof3/bof3.h"

extern u8 D_80143F03;
extern u8 D_80146866;
extern u16* D_80148210; /* @source 0x80148210 @kind unknown */

/* @source 0x801F43B8
 * @behavior Overlay handler: advances the halfword at offset 0x0A of the work
 *           record published through the global pointer cell 0x80148210 by 3
 *           when the front-side byte D_80143F03 reads 2 while the shared mode
 *           byte D_80146866 is clear, and again when D_80143F03 reads 1. The
 *           front-side byte is re-read for the second test, and both blocks use
 *           the same three instructions on the published halfword. Takes no
 *           arguments, returns nothing and uses no stack frame.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceHalfword0aBy3InFrontByte1OrUngated2World04Area16713_801F43B8(void) {
  if (D_80143F03 == 2 && D_80146866 == 0) {
    D_80148210[5] += 3;
  }

  if (D_80143F03 == 1) {
    D_80148210[5] += 3;
  }
}
