#include "bof3/bof3.h"

/* Shared battle work pointer in main RAM, outside this overlay's image; Splat
 * binds the address through this encoded name, which is why the original folds
 * the access to lui plus %lo. The sibling bmagic lifts read the same record:
 * incrementWorkByteAThenSetGlobalWorkByteBToFFAt80AndInvokeHelperBmagicMagic
 * 10803_801EF130 writes its byte 0xB. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Scratchpad work-object pointer cell. Declared as a named fixed address so
 * every access re-materializes the cell with its own lui plus offset load,
 * which is how the original reaches it once per group. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind unknown */

/* @source 0x801EF7D0
 * @behavior Per-frame handler of this overlay's handler run. It adds 0x10 to
 *           the scratchpad work object's three colour channels at bytes
 *           0x5D/0x5E/0x5F, reading the work-object pointer cell at 0x1F800044
 *           once per channel, then reads the channel at 0x5D back through a
 *           signed byte view. Only when that red channel is zero does it set
 *           byte 0xB of the shared battle work record read through 0x801EB4E0
 *           to 0xFF, re-read the work-object pointer cell and increment the
 *           object's handler byte 2 with a byte-width add, advancing the
 *           dispatcher on work byte 2 to its next handler step. The cell is
 *           re-read before that final group because a store through the record
 *           pointer may alias it, and the 0xFF lands in the branch delay slot
 *           that the original fills while the channel test resolves. Leaf with
 *           no frame; takes no arguments and returns nothing, and reads or
 *           writes no other state. The colour group copies the complete exact
 *           sibling addWorkColorTripletBy16ThenDecrementGlobalWorkByteBAnd
 *           AdvanceWorkByte2BmagicMagic01500_801F1180, which tests the same
 *           channel through the same signed byte view; this function's own
 *           zero test and 0xFF store replace that sibling's -0x40 test and
 *           byte 0xB decrement.
 * @status exact
 * @match 100.00
 * @residual none
 */
void addWorkColorTripletBy16ThenSetGlobalWorkByteBToFFAndAdvanceWorkByte2WhenChannelIsZeroBmagicMagic02003_801EF7D0(void) {
  D_1F800044[0x5D] += 0x10;
  D_1F800044[0x5E] += 0x10;
  D_1F800044[0x5F] += 0x10;

  if (((s8 *)D_1F800044)[0x5D] == 0) {
    D_801EB4E0[0xB] = 0xFF;
    D_1F800044[2]++;
  }
}
