#include "bof3/bof3.h"

/* This overlay's step handler run: twenty-four in-image code pointers from
 * 0x801D2C48 through 0x801D32E0, filling the head of the reviewed `textbin` row
 * T_801D9688, which the Splat layout places, so the run is only referenced here.
 * This dispatcher indexes it from 0x801D9720, the run's first word, so the
 * reachable entries are 0x801D2C48 through 0x801D32E0. */
extern void (*D_801D9720[])(void); /* @source 0x801D9720 @kind data */

/* The overlay's state byte, part of the work block 0x801DA028..0x801DA12D:
 * initializeShisuUiWorkRecordsShisu00_801D27BC clears it while setting up that
 * block, and this dispatcher turns its value into a handler index. No other
 * code of this payload writes it, so the steps that advance it live outside
 * the overlay. */
extern u8 D_801DA049; /* @source 0x801DA049 @kind data */

/* @source 0x801D2C0C
 * @behavior Overlay step dispatch on the SHISU state byte at 0x801DA049: takes
 *           that byte, scales it by four and calls the matching entry of this
 *           overlay's handler run based at 0x801D9720 with no arguments,
 *           discarding any result. The original does not bound the index before
 *           the scaled table load. It reads no other state and writes none; the
 *           byte load precedes frame creation, the 0x18-byte frame only keeps
 *           $ra at 0x10 across the indirect call, and the slot after the table
 *           load stays a nop.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchStateByteTable9720EtcShisu00_801D2C0C(void)
{
    D_801D9720[D_801DA049]();
}
