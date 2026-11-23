#include "bof3/bof3.h"

/* Shared state entry this overlay advances and clears. */
extern u8 D_801448EB; /* @source 0x801448EB @kind data */

/* Shared progress byte that steers this overlay's front-end sequence. The
 * dispatch reads it with `lb`, so the signed read carries a cast. */
extern u8 D_801448EC[]; /* @source 0x801448EC @kind data */

/* Shared progress byte of the released front-end work. */
extern u8 D_80146867[]; /* @source 0x80146867 @kind data */

void func_8015C058(void);
void func_8015C088(void);

/* @source 0x801F2C04
 * @behavior Steps the shared progress byte 0x801448EC through this overlay's
 *           front-end arm and release. While that byte reads 0 the body starts
 *           the shared front-end through func_8015C088 and advances the byte to
 *           1. While it reads 1 the body waits for the shared progress byte
 *           0x80146867 to read 2 and then releases the overlay through
 *           func_8015C058, clears 0x80146867, clears the shared state entry byte
 *           0x801448EB and returns the progress byte to 0. Any other progress
 *           value is left untouched. Takes no arguments and returns nothing.
 *
 * Shape: the switch topology and the release tail follow the reviewed sibling
 *        stepSharedModeStartAction1ThenReleaseOverlayOnPhaseChangeWorld02Area10513_801F3180,
 *        which carried the same switch on the signed byte 0x801448EC and the
 *        same release tail of func_8015C058 plus the cleared 0x801448EB, and
 *        whose gate measured exact. The reviewed sibling func_801F2C38 of
 *        world00/area016/13 gave the dispatch skeleton and the cell
 *        representation: `lb` of 0x801448EC through a signed cast of its array
 *        binding, a compare against 0 and 1 with the case constants
 *        materialised in the taken-branch delay slots, a plain jump to the
 *        shared epilogue for every other value, and both cells held in one base
 *        register each so the scalar access is written as element 0. This
 *        body's own literals are progress 1 (the area105 sibling incremented
 *        it), a release arm gated on 0x80146867 reading 2 rather than on the
 *        area105 sibling's phase byte leaving 2, and a release tail that clears
 *        0x80146867 as well as 0x801448EB and the progress byte. A whole-store
 *        scan of the 398 extracted payloads finds no byte-identical copy of
 *        these 0x94 bytes, and this payload carries neither the address word
 *        0x801F2C04 nor a `jal` encoding that reaches it, so its caller sits
 *        outside the payload and every role stays structural.
 * @status exact
 * @match 100.00
 * @residual none
 */
void audioReleaseOverlayProgress(
    void) {
  switch (*(s8*)D_801448EC) {
  case 0:
    func_8015C088();
    D_801448EC[0] = 1;
    break;
  case 1:
    if (D_80146867[0] == 2) {
      func_8015C058();
      D_80146867[0] = 0;
      D_801448EB = 0;
      D_801448EC[0] = 0;
    }
    break;
  }
}
