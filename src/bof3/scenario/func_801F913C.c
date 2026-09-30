#include "bof3/scenario/scena00_internal.h"

/* @source 0x801F913C
 * @behavior Dispatches one frame of the overlay's state machine: it reads the
 * signed main-RAM state byte D_80146872, selects the matching entry of the
 * overlay-local handler table at 0x801FCA10 (entries 0 through 14 are this
 * overlay's state handlers 0x801F9178 through 0x801FB924) and invokes it; no
 * other state is read and none is written. Takes no arguments and returns
 * nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801F913C(void) {
  D_801FCA10[D_80146872]();
}
