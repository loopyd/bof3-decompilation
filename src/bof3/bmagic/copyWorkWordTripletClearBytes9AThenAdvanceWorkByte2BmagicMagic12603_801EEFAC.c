#include "bof3/bof3.h"

extern u8 *D_1F800044; /* @source 0x1F800044 @kind data */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind data */

/* @source 0x801EEFAC
 * @behavior Work-object step handler. Copies the record words at 0x34, 0x38 and
 * 0x3C, read through the shared record pointer at 0x801EB4E0, into the
 * scratchpad work object published at the pointer cell 0x1F800044; clears work
 * byte 9 and work byte 0xA; then increments work byte 2. The third record word
 * is held in a local so its load precedes the byte 9 clear and its store
 * follows it, and the work pointer cell is re-read before the byte 0xA clear and
 * again before the byte 2 increment, because a store through the loaded pointer
 * may alias the cell. Leaf with no frame; takes no arguments and returns
 * nothing. Field roles stay offset-based.
 * @status exact
 * @match 100.00
 * @residual none
 */
void copyWorkWordTripletClearBytes9AThenAdvanceWorkByte2BmagicMagic12603_801EEFAC(void) {
    u8 *source;
    u8 *work;
    u8 *next;
    u8 *step;
    u32 value;

    source = D_801EB4E0;
    work = D_1F800044;
    *(u32 *)(work + 0x34) = *(u32 *)(source + 0x34);
    *(u32 *)(work + 0x38) = *(u32 *)(source + 0x38);
    value = *(u32 *)(source + 0x3C);
    work[9] = 0;
    next = D_1F800044;
    *(u32 *)(work + 0x3C) = value;
    next[0xA] = 0;
    step = D_1F800044;
    step[2]++;
}
