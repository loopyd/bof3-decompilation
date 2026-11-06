#include "bof3/bof3.h"

/* This overlay's handler run starts at 0x801D1750 and holds nine in-image code
 * pointers, entries 0x801D0C5C through 0x801D1628, followed by the state byte at
 * 0x801D1774. The target's `textbin` row T_801D1750 places the whole run, so it is
 * only referenced here. This dispatcher indexes it from 0x801D1758, its third
 * entry, so the reachable entries are 0x801D0C98 through 0x801D1628. */
extern u8 D_801F294D; /* @source 0x801F294D @kind data */
extern void (*D_801D1758[])(void); /* @source 0x801D1758 @kind data */

/* @source 0x801D0C5C
 * @behavior Overlay step dispatch on the byte at 0x801F294D: loads that unsigned
 * byte, scales it by four and calls the matching entry of this overlay's handler
 * run based at 0x801D1758 with no arguments, discarding any result. The original
 * does not bound the index before the scaled table load. It reads no other state
 * and writes none; the 0x18-byte frame only keeps $ra at 0x10 across the indirect
 * call, the index load's delay slot takes the frame setup and the slot after the
 * table load stays a nop. The byte is the overlay's step counter: the neighbouring
 * handlers 0x801D0C98 and 0x801D0CF4 advance it by one as they run. Its only
 * in-image reference is the word at 0x801D1750, entry 0 of that run, which the
 * target's other dispatcher func_801D0C04 indexes from 0x801F294C, so it has no
 * jal caller and takes no arguments.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchByte1f294dTable1758Commu0406_801D0C5C(void) {
    D_801D1758[D_801F294D]();
}
