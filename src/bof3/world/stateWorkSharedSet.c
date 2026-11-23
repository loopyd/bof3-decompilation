#include "bof3/bof3.h"

/* Shared overlay state byte of the main executable, outside this overlay's
 * payload. Splat binds it through this target's generated undefined-syms map,
 * so no target-map row is needed. */
extern u8 D_8014933B; /* @source 0x8014933B @kind data */

/* Work record published through the scratchpad pointer cell 0x1F800044. The
 * target map already binds that cell, so this source reuses its name. */
extern u8* world03Area11913WorkRecord; /* @source 0x1F800044 @kind data */

/* @source 0x801F35D0
 * @behavior Overlay step handler: when the shared overlay state byte at
 *           0x8014933B is nonzero it increments the work record's state byte at
 *           offset 0x02, which is the byte the overlay's byte-2 dispatchers
 *           index; while that shared byte reads zero the step leaves the record
 *           untouched. The work record comes from the scratchpad pointer cell
 *           0x1F800044, read only on the taken path, and the byte is loaded and
 *           stored back through that pointer, so the increment wraps at 0x100.
 *           Takes no arguments, calls nothing and returns nothing; leaf with no
 *           frame.
 *           This is the first entry of the two-entry handler table at
 *           0x801F4604, whose dispatcher callStateTable4604World
 *           indexes it by work byte 2, so the increment moves the dispatch on to
 *           the second entry at 0x801F3608, the step that draws the overlay's
 *           animation. The shared state byte is the same cell the exact sibling
 *           stateCameraOutsideFixed
 *           reads at 0x801F3170, and the work-record binding follows the exact
 *           sibling stateWorkQueueAdvance,
 *           which performs the same byte-2 increment through the same pointer
 *           cell.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateWorkSharedSet(void) {
  u8* work;

  if (D_8014933B != 0) {
    work = world03Area11913WorkRecord;
    work[2] += 1;
  }
}
