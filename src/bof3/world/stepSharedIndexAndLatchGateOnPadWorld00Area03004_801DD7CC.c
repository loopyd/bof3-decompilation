#include "bof3/world/area03004_internal.h"

/* @source 0x801DD7CC
 * @behavior AREA030 pad handler (entry 2 of the 0x801E22E4 dispatch table
 * selected by the shared byte D_80144286): it first submits the fixed
 * func_8014F800 panel texture row (0x1D, 0x14, 0, 0xFF, 0x80010000 + the
 * main-executable halfword D_800100E0). Then, only while the shared state byte
 * D_80144281 holds 0x0E and the shared pad gate byte 0x80144288 is clear, it
 * reacts to the shared pad state D_80145AA8: a bit shared with the pressed
 * mask D_80145AC4 dispatches cue 0x106, clears byte 4 of the work record
 * published at the scratchpad cursor 0x1F800044 and decrements that record's
 * dispatch byte 3; the Cross bit 0x4000 publishes 0xFF into the gate byte and
 * steps the shared index D_801442BC up over the 0..5 range; the Triangle bit
 * 0x1000 publishes 1 into the gate byte and steps that index down, wrapping to
 * 5 below zero.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stepSharedIndexAndLatchGateOnPadWorld00Area03004_801DD7CC(void) {
    u8* work;

    func_8014F800(0x1D, 0x14, 0, 0xFFu, 0x80010000u + (u32)D_800100E0);
    if (D_80144281 != 0x0E || gateByteWorld00Area03004_80144288 != 0) {
        return;
    }
    if (D_80145AA8 & D_80145AC4) {
        func_8015DF18(0x106);
        D_1F800044[4] = 0;
        work = D_1F800044;
        work[3] = work[3] - 1;
    } else if (D_80145AA8 & 0x4000) {
        gateByteWorld00Area03004_80144288 = 0xFF;
        D_801442BC++;
        if (D_801442BC >= 6) {
            D_801442BC = 0;
        }
    } else if (D_80145AA8 & 0x1000) {
        gateByteWorld00Area03004_80144288 = 1;
        D_801442BC--;
        if ((s16)D_801442BC < 0) {
            D_801442BC = 5;
        }
    }
}
