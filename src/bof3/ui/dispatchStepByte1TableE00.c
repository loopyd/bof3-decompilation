#include "bof3/bof3.h"

/* Overlay-wide step cell group. Byte 0 indexes the five-entry handler run at
 * 0x801DADEC through dispatchStepByte0TableDec; byte 1 selects a handler of the
 * run entered at 0x801DAE00. */
extern u8 D_801F294C[]; /* @source 0x801F294C @kind data */

/* Handler-pointer run entered at 0x801DAE00, four-byte stride. */
extern void (*D_801DAE00[])(void); /* @source 0x801DAE00 @kind table */

/* @source 0x801D5DF4
 * @behavior Second-level dispatcher of the overlay step group: reads the
 * unsigned byte at 0x801F294D - byte 1 of the step cell group at 0x801F294C -
 * scales it by four, loads the handler pointer that run selects from the
 * 0x801DAE00 base and calls it with no arguments, discarding the result. The
 * index is not bounded before the scaled table load and the function reads and
 * writes no other state; the 0x18-byte frame only keeps $ra at 0x10 across the
 * indirect call, the index load's delay slot takes the frame setup and the
 * slots after the table load and the indirect call stay nops. Entry 1 of the
 * 0x801DADEC byte-0 run at 0x801DADF0 is the only reference the payload makes
 * to it, and that slot is called as a no-argument handler, so its parameter
 * roles stay structural.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchStepByte1TableE00(void) {
    D_801DAE00[D_801F294C[1]]();
}
