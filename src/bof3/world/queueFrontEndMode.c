#include "bof3/bof3.h"

/* Front-end mode byte of the shared main executable. The exact lift at
 * setScratchWorkByte2WhenFrontEndModeNot5World03Area14514_801F2E2C reads the
 * same byte against the same constant 5, and the scenario loader waits on that
 * byte against its ready phase, so the cell keeps its address-anchored name.
 * @source 0x80143BB0 @kind unknown
 */
extern u8 D_80143BB0;

extern void func_8015DF18(u32 cue); /* @source 0x8015DF18 */

/* @source 0x801F3BEC
 * @behavior Overlay front-end cue gate: reads the shared front-end mode byte
 *           D_80143BB0 and, unless that byte reads exactly 5, queues sound cue
 *           0x201 through the main-executable dispatcher 0x8015DF18. The
 *           compared constant 5 is materialised before the test, the saving of
 *           the return address fills that branch's delay slot, and the cue id
 *           is materialised in the call's delay slot, so the 0x18-byte frame
 *           holds only the saved return address; it takes no arguments and
 *           returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void queueFrontEndMode(void) {
  if (D_80143BB0 != 5) {
    func_8015DF18(0x201);
  }
}
