#include "bof3/bof3.h"

/* Overlay-wide step cell group. Byte 0 indexes the ten-entry handler run at
 * 0x801DAD68 through dispatchStepHandler; byte 1 selects a handler of the same
 * run through the views entered at 0x801DAD74 and 0x801DAD7C. */
extern u8 D_801F294C[]; /* @source 0x801F294C @kind data */

/* Handler-pointer run entered at stepHandlerTable + 3 (0x801DAD74). */
extern void (*D_801DAD74[])(void); /* @source 0x801DAD74 @kind table */

/* @source 0x801D5070
 * @behavior Second-level dispatcher of the overlay step group: reads the
 * unsigned byte at 0x801F294D, uses it to index the handler-pointer run at
 * 0x801DAD74 - entry three of the same ten-entry run the byte 0 dispatcher
 * reads at 0x801DAD68 - and calls the selected handler with no arguments.
 * The function stores nothing, reads nothing else, restores $ra/$sp and uses
 * no handler result.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchStepByte1TableD74(void) {
    D_801DAD74[D_801F294C[1]]();
}
