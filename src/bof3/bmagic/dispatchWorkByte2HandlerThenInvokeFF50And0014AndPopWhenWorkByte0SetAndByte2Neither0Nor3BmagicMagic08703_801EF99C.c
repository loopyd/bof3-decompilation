#include "bof3/bof3.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */
extern void (*D_801F0530[])(void); /* @source 0x801F0530 @kind data */
extern void func_801EFF50(void); /* @source 0x801EFF50 @kind function */
extern void func_801F0014(void); /* @source 0x801F0014 @kind function */

/* @source 0x801EF99C
 * @behavior Calls the handler indexed by unsigned work-record byte 2 through
 *           the pointer table at 0x801F0530, without an index bound. The work
 *           pointer comes from scratchpad cell 0x1F800044. After the call,
 *           reloads that cell and, when byte 0 is nonzero and byte 2 is neither
 *           0 nor 3, calls helpers 0x801EFF50 and 0x801F0014, then PopMatrix.
 *           Takes no arguments and returns nothing. Helper roles remain
 *           address-anchored. The 0x18-byte frame preserves only the return
 *           address; the function performs no direct stores to the record.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte2HandlerThenInvokeFF50And0014AndPopWhenWorkByte0SetAndByte2Neither0Nor3BmagicMagic08703_801EF99C(void) {
    u8 *step_record;
    u8 *active_record;

    step_record = g_battle_work;
    D_801F0530[step_record[2]]();
    active_record = g_battle_work;
    if (active_record[0] != 0 && active_record[2] != 0 && active_record[2] != 3) {
        func_801EFF50();
        func_801F0014();
        PopMatrix();
    }
}
