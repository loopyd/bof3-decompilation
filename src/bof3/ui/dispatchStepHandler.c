#include "bof3/bof3.h"

/* Overlay-wide step cell group. Byte 0 indexes the ten-entry handler table
 * below and is advanced by the handlers at 0x801D50E4 and 0x801D5A48; byte 1
 * indexes the views of the same run at 0x801DAD74 and 0x801DAD7C. */
extern u8 D_801F294C[]; /* @source 0x801F294C @kind data */

/* Ten handler pointers, 0x801D5070 through 0x801D5A48, sitting in the trailing
 * data blob; the same run is entered at +0xC and +0x14 by the byte-1 views. */
extern void (*stepHandlerTable[])(void); /* @source 0x801DAD68 @kind table */

/* @source 0x801D5034
 * @behavior Reads the unsigned byte at 0x801F294C, uses it to index the
 * ten-entry handler table at 0x801DAD68 and calls the selected handler with no
 * arguments. The handlers of that table advance the step byte or run one step
 * of the overlay sequence; this dispatcher stores nothing, reads nothing else
 * and passes the handler's result through untouched.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchStepHandler(void) {
    stepHandlerTable[D_801F294C[0]]();
}
