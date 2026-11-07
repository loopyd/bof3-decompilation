#include "bof3/bof3.h"

#include <stdlib.h>

/* Shared camera coordinate words of the main executable, both outside this
 * overlay's payload. Splat binds them through this target's generated
 * undefined-syms map, so no target-map row is needed. */
extern s32 D_80145EC4; /* @source 0x80145EC4 @kind data */
extern s32 D_80145EC8; /* @source 0x80145EC8 @kind data */

/* Shared overlay state byte of the main executable, outside this overlay's
 * payload. This step only reads it, so it is bound with a plain byte extern. */
extern u8 D_8014933B; /* @source 0x8014933B @kind data */

/* @source 0x801F3170
 * @behavior Overlay step handler: when the shared overlay state byte at
 *           0x8014933B reads zero it compares the shared camera coordinate
 *           words with this overlay's fixed coordinate window and, when the
 *           camera lies outside that window, stores 4 into the work record's
 *           state byte at offset 0x02, the byte the overlay's byte-2
 *           dispatcher indexes. The camera is outside when the absolute
 *           difference between D_80145EC4 and 0x428000 exceeds 0x18000, or
 *           when the absolute differences between D_80145EC8 and both
 *           0x3E0000 and 0x3F0000 exceed 0x18000. The original materialises
 *           0x18000 once and reuses it for all three comparisons, and it reads
 *           D_80145EC8 once for the two tests that share it. The work record
 *           comes from the scratchpad pointer cell 0x1F800044, which is read
 *           once, after the comparison chain. Takes no arguments, calls
 *           nothing and returns nothing; leaf with no frame.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setWorkState2To4WhenCameraOutsideFixedCoordinateWindowWorld03Area11913_801F3170(
    void) {
  u8* work;

  if (D_8014933B != 0) {
    return;
  }
  if (abs(D_80145EC4 - 0x428000) > 0x18000 ||
      (abs(D_80145EC8 - 0x3E0000) > 0x18000 &&
       abs(D_80145EC8 - 0x3F0000) > 0x18000)) {
    work = *(u8**)SPAD_ADDR(u8, 0x44u);
    work[2] = 4;
  }
}
