#include "bof3/scenario/scena00_internal.h"

/* @source 0x801F9B98
 * @behavior Dispatches one frame of the overlay's primary state machine from
 * the signed main-RAM state byte D_80146874: that byte is scaled by four and
 * selects an entry of the overlay-local secondary handler table at 0x801FCA1C
 * (whose first entries are the empty handlers 0x801F9BD4 and 0x801F9BDC,
 * followed by the primary controller 0x801F9BE4 and the middle controllers),
 * and the selected entry is invoked; no state is read other than that byte and
 * none is written. Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801F9B98(void) {
  D_801FCA1C[D_80146874]();
}
