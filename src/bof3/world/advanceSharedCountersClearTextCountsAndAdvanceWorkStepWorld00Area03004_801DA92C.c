#include "bof3/world/area03004_internal.h"

/* @source 0x801DA92C
 * @behavior AREA030 mode-entry sharedStep: stores 1 in the shared state byte
 * D_8014419E, increments the shared sharedStep counter byte D_8014403E, advances the
 * shared selection counter D_80144199_BYTE, stores 1 in byte 6 of the dispatchWork
 * record published at the scratchpad cursor 0x1F800044, clears the four
 * D_801E31E0 slot character counters, clears record byte 0x07 and (before the
 * func_8014D8D4(3) call) record byte 0x0B, then increments record byte 3.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceSharedCountersClearTextCountsAndAdvanceWorkStepWorld00Area03004(void) {
    u8 sharedStep;
    u8 sharedCount;
    u8* dispatchWork;
    u8* textCountPtr;
    u8* sharedStepByte;
    u8* flagWork;
    s32 one;
    s32 textSlotIndex;

    textSlotIndex = 3;
    textCountPtr = D_801E31E0 + 3;
    sharedStepByte = D_8014403E;
    do {
        sharedStep = sharedStepByte[0];
        sharedCount = D_80144199_BYTE;
        do {
            flagWork = D_1F800044;
        } while (0);
    } while (0);
    one = 1;
    D_8014419E = one;
    do {
        sharedStep++;
        do {
            sharedCount++;
        } while (0);
    } while (0);
    sharedStepByte[0] = sharedStep;
    D_80144199_BYTE = sharedCount;
    flagWork[6] = one;
    for (; textSlotIndex >= 0; textSlotIndex--) {
        *textCountPtr-- = 0;
    }
    D_1F800044[7] = 0;
    D_1F800044[0x0B] = 0;
    func_8014D8D4(3);
    dispatchWork = D_1F800044;
    dispatchWork[3]++;
}
