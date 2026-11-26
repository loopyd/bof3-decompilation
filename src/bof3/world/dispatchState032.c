#include "bof3/bof3.h"
#include "bof3/emi/dispatchers.h"

extern void (*D_801F67F0[])(void);

/* @source 0x801F36A4
 * @behavior Overlay step dispatch: reads the work-record state byte at offset
 * 0x03 of the record published at the scratchpad pointer slot 0x44 and calls
 * the handler pointer at that byte index of the second local handler table
 * 0x801F67F0. Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(dispatchState03, dispatchState03, D_801F67F0);
