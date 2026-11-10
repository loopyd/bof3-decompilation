#include "bof3/bof3.h"

extern u8 *D_801F22BC; /* @source 0x801F22BC @kind data */
extern void (*D_801F12AC[])(void); /* @source 0x801F12AC @kind data */

extern void func_801F11AC(void); /* @source 0x801F11AC @kind function */
extern void func_801F0ED0(void); /* @source 0x801F0ED0 @kind function */

/* @source 0x801F0960
 * @behavior Reads the record pointer at 0x801F22BC and calls the handler
 * indexed by its unsigned byte 2 in the table based at 0x801F12AC, without
 * an index bound. Reloads the record pointer after dispatch and tests bytes
 * 0 and 2 in that order. When both are nonzero, calls 0x801F11AC followed
 * by 0x801F0ED0. All calls pass no arguments and discard their results.
 * This callback returns nothing and writes no state directly.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchRecordByte2ThenHelpersWhenBytes0And2SetBmagicMagic12603_801F0960(void) {
    u8 *step_record;
    u8 *gate_record;

    step_record = D_801F22BC;
    D_801F12AC[step_record[2]]();
    gate_record = D_801F22BC;
    if (gate_record[0] != 0 && gate_record[2] != 0) {
        func_801F11AC();
        func_801F0ED0();
    }
}
