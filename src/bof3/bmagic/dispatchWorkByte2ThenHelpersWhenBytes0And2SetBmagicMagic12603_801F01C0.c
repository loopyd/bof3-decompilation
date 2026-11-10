#include "bof3/bof3.h"

extern u8 *D_1F800044; /* @source 0x1F800044 @kind data */
extern void (*D_801F1288[])(void); /* @source 0x801F1288 @kind data */

extern void func_800AF764(void); /* @source 0x800AF764 @kind function */
extern void func_801F06AC(void); /* @source 0x801F06AC @kind function */

/* @source 0x801F01C0
 * @behavior Reads the scratchpad work pointer at 0x1F800044 and calls the
 * handler indexed by its unsigned byte 2 in the table at 0x801F1288,
 * without an index bound. Reloads the work pointer after dispatch and
 * tests bytes 0 and 2 in that order. When both are nonzero, calls
 * 0x800AF764 followed by 0x801F06AC. All calls pass no arguments and
 * discard their results. This callback returns nothing and writes no
 * state directly. The table at 0x801F125C holds this entry at 0x801F1264.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte2ThenHelpersWhenBytes0And2SetBmagicMagic12603_801F01C0(void) {
    u8 *step_record;
    u8 *gate_record;

    step_record = D_1F800044;
    D_801F1288[step_record[2]]();
    gate_record = D_1F800044;
    if (gate_record[0] != 0 && gate_record[2] != 0) {
        func_800AF764();
        func_801F06AC();
    }
}
