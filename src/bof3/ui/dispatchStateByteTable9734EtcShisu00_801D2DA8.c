#include "bof3/bof3.h"

/* This overlay's second step handler run: twenty-four in-image code pointers
 * from 0x801D2C48 through 0x801D32E0, filling the head of the reviewed
 * `textbin` row T_801D9688, which the Splat layout places, so the run is only
 * referenced here. This dispatcher indexes it from the run's sixth word at
 * 0x801D9734, so the reachable entries are 0x801D2DE4 through 0x801D32E0.
 */
extern void (*D_801D9734[])(void); /* @source 0x801D9734 @kind data */

/* The overlay state byte, part of the work block 0x801DA028..0x801DA12D:
 * func_801D27BC clears it while setting up that block, and this dispatcher turns
 * its value into a handler index. No other code of this payload writes it, so
 * the steps that advance it live outside the overlay.
 */
extern u8 D_801DA0C1; /* @source 0x801DA0C1 @kind data */

/* @source 0x801D2DA8
 * @behavior Handler dispatch on the SHISU state byte at 0x801DA0C1: takes that
 *           byte, scales it by four and calls the matching entry of this
 *           overlay's handler run based at 0x801D9734 with no arguments. The
 *           byte is not bounded before the scaled table load, and the selected
 *           handler's result is left in $v0 by the epilogue. It reads no other
 *           state and writes none; the byte load precedes frame creation, the
 *           0x18-byte frame only keeps $ra at 0x10 across the indirect call, and
 *           the slot after the table load stays a nop.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchStateByteTable9734EtcShisu00_801D2DA8(void)
{
    D_801D9734[D_801DA0C1]();
}
