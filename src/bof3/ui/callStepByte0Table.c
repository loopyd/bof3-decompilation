#include "bof3/bof3.h"

/* Overlay-wide step cell group. Byte 0 indexes the ten-entry run at
 * 0x801DAD68 through dispatchStepHandler and the five-entry run below; byte 1
 * selects a handler of the 0x801DAE00 run through callStepByte1Table
 * and the later views of that run. */
extern u8 D_801F294C[]; /* @source 0x801F294C @kind data */

/* Five handler pointers, 0x801DADEC through 0x801DADFC, in this overlay's
 * trailing textbin run: the step 0 handler 0x801D5D98 and the byte-1 dispatch
 * views 0x801D5DF4, 0x801D6CDC, 0x801D7DD4 and 0x801D7E30. */
extern void (*D_801DADEC[])(void); /* @source 0x801DADEC @kind table */

/* @source 0x801D5D5C
 * @behavior Reads the unsigned byte at 0x801F294C, scales it by four and calls
 * the handler the five-entry run at 0x801DADEC selects, discarding any result.
 * The index is not bounded before the scaled table load and the function reads
 * and writes no other state; the 0x18-byte frame only keeps $ra at 0x10 across
 * the indirect call, the index load's delay slot takes the frame setup and the
 * slots after the table load and the indirect call stay nops. No jal and no
 * pointer word in the payload names it, so the overlay loader is its only
 * caller and its parameter roles stay structural.
 * @status exact
 * @match 100.00
 * @residual none
 */
void callStepByte0Table(void) {
    D_801DADEC[D_801F294C[0]]();
}
