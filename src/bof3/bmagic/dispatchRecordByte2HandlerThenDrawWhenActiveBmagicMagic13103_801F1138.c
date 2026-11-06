#include "bof3/bof3.h"

extern u8 *workRecord801F24E0; /* @source 0x801F24E0 @kind data */

/* Three-entry overlay handler table for work-record byte 2. */
extern void (*dispatchHandlerTable801F1C48[])(void); /* @source 0x801F1C48 @kind data */

/* Projects the work record's position and stores the screen pair. */
extern void func_801F1B14(void);

/* Draws the eight-step effect of the active work record. */
extern void func_801F1844(void);

/* @source 0x801F1138
 * @behavior Reads the record pointer cell at 0x801F24E0 and uses the record
 * byte at offset 0x2, scaled by four, to select one entry of the three-entry
 * handler table at 0x801F1C48, which it calls with no arguments. When the
 * handler returns it reads the cell again in a second load; if the record byte
 * at offset 0x0 and the byte at offset 0x2 are both non-zero it calls
 * func_801F1B14 and then func_801F1844. Reads no other state, stores nothing
 * and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchRecordByte2HandlerThenDrawWhenActiveBmagicMagic13103_801F1138(void) {
    u8 *step_record;
    u8 *gate_record;

    step_record = workRecord801F24E0;
    dispatchHandlerTable801F1C48[step_record[2]]();
    gate_record = workRecord801F24E0;
    if (gate_record[0] != 0 && gate_record[2] != 0) {
        func_801F1B14();
        func_801F1844();
    }
}
