#include "bof3/bof3.h"

extern u8 *workRecord801F24E0; /* @source 0x801F24E0 @kind data */

/* Three-entry overlay handler table for work-record byte 2. */
extern void (*dispatchHandlerTable801F1C94[])(void); /* @source 0x801F1C94 @kind data */

/* Projects the work record's position and stores the screen pair. */
extern void func_801F1B14(void);

/* Draws the eight-step effect of the active work record. */
extern void func_801F1844(void);

/* @source 0x801F14A0
 * @behavior Reads the record pointer cell at 0x801F24E0 and uses the record
 * byte at offset 0x2, scaled by four, to select one entry of the three-entry
 * handler table at 0x801F1C94, which it calls with no arguments. When the
 * handler returns it reads the cell again in a second load and loads the signed
 * halfword at offset 0x26: when that halfword is non-zero it stores the value
 * minus one there and jumps straight to the epilogue, calling nothing else.
 * When the halfword is zero and the record bytes at offsets 0x0 and 0x2 are
 * both non-zero it calls func_801F1B14 and then func_801F1844. Reads no other
 * state, stores nothing but that halfword and returns nothing. The halfword's
 * wider role is unknown.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchRecordByte2HandlerThenDecrementCountdownOrDrawWhenActiveBmagicMagic13103_801F14A0(void) {
    u8 *step_record;
    u8 *countdown_record;
    s16 countdown;

    step_record = workRecord801F24E0;
    dispatchHandlerTable801F1C94[step_record[2]]();
    countdown_record = workRecord801F24E0;
    countdown = *(s16 *)(countdown_record + 0x26);
    if (countdown != 0) {
        *(s16 *)(countdown_record + 0x26) = countdown - 1;
    } else if (countdown_record[0] != 0 && countdown_record[2] != 0) {
        func_801F1B14();
        func_801F1844();
    }
}
