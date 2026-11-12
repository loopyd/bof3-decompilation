#include "bof3/world/area03004_internal.h"

/* @source 0x801DD664
 * @behavior AREA030 pad handler (entry 1 of the 0x801E22E4 dispatch table
 * selected by the shared byte D_80144286): it first submits the fixed
 * func_8014F800 panel texture row (0x1D, 0x14, 0, 0xFF, 0x80010000 + the
 * main-executable halfword D_800100C2). Then, only while the shared state byte
 * D_80144281 holds 8, it reacts to the shared pad state D_80145AA8: the Up bit
 * 0x2000 dispatches cue 0x102, publishes the halfword at D_801442BE plus one
 * into the shared index D_801442BC, wraps that index to 0 above 0x16,
 * publishes 0 into the shared pad gate byte 0x80144288 and advances the state
 * byte; the Down bit 0x8000 dispatches cue 0x102, publishes that halfword
 * minus one into D_801442BC, wraps the index to 0x16 below zero, publishes 1
 * into the gate byte and advances the state byte; otherwise a bit shared with
 * the pressed mask D_80145AC4 dispatches cue 0x106, clears byte 4 of the work
 * record published at the scratchpad cursor 0x1F800044 and decrements that
 * record's dispatch byte 3.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stepSharedIndexLatchGateOnPadAndAdvanceStateWorld00Area03004_801DD664(
    void) {
    u8* work;
    u8* state;

    func_8014F800(0x1D, 0x14, 0, 0xFFu, 0x80010000u + (u32)D_800100C2);
    state = &D_80144281;
    if (*state != 8) {
        return;
    }
    if (D_80145AA8 & 0x2000) {
        func_8015DF18(0x102);
        D_801442BC = D_801442BE + 1;
        if (D_801442BC >= 0x17) {
            D_801442BC = 0;
        }
        gateByteWorld00Area03004_80144288 = 0;
        *state = *state + 1;
    } else if (D_80145AA8 & 0x8000) {
        func_8015DF18(0x102);
        D_801442BC = D_801442BE - 1;
        if ((s16)D_801442BC < 0) {
            D_801442BC = 0x16;
        }
        gateByteWorld00Area03004_80144288 = 1;
        *state = *state + 1;
    } else if (D_80145AA8 & D_80145AC4) {
        func_8015DF18(0x106);
        D_1F800044[4] = 0;
        work = D_1F800044;
        work[3] = work[3] - 1;
    }
}
