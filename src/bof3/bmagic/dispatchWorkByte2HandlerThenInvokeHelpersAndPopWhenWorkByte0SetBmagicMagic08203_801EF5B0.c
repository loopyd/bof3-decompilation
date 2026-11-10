#include "bof3/bof3.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */
extern void (*D_801F0D1C[])(void); /* @source 0x801F0D1C @kind data */

extern void func_801EF4BC(void); /* @source 0x801EF4BC @kind function */
extern void func_801EFA00(void); /* @source 0x801EFA00 @kind function */
extern void func_801EF720(void); /* @source 0x801EF720 @kind function */

/* @source 0x801EF5B0
 * @behavior Dispatches the handler selected by unsigned work-record byte 2
 *           through the table at 0x801F0D1C, without an index bounds check.
 *           The record pointer comes from the scratchpad cell at 0x1F800044.
 *           After the handler returns, reads that cell again and tests record
 *           byte 0. When nonzero, calls the local helpers at 0x801EF4BC,
 *           0x801EFA00 and 0x801EF720 in that order, then calls PopMatrix.
 *           Takes no arguments and returns nothing. The first local helper
 *           pushes the matrix restored by this PopMatrix. The 0x18-byte frame
 *           preserves only the return address. This function is itself named
 *           by the pointer at 0x801F0D14 in the byte-1 dispatch table.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte2HandlerThenInvokeHelpersAndPopWhenWorkByte0SetBmagicMagic08203_801EF5B0(void) {
    u8 *step_record;
    u8 *active_record;

    step_record = g_battle_work;
    D_801F0D1C[step_record[2]]();
    active_record = g_battle_work;
    if (active_record[0] != 0) {
        func_801EF4BC();
        func_801EFA00();
        func_801EF720();
        PopMatrix();
    }
}
