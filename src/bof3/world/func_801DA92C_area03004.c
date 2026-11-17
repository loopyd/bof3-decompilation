#include "bof3/world/area03004_internal.h"

/* @source 0x801DA92C
 * @behavior AREA030 mode-entry step: stores 1 in the shared state byte
 * D_8014419E, increments the shared step counter byte D_8014403E, advances the
 * shared selection counter D_80144199_BYTE, stores 1 in byte 6 of the work
 * record published at the scratchpad cursor 0x1F800044, clears the four
 * D_801E31E0 slot character counters, clears record byte 0x07 and (before the
 * func_8014D8D4(3) call) record byte 0x0B, then increments record byte 3.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801DA92C(void) {
    u8 step;
    u8 count;
    u8* work;
    u8* p;
    u8* stepBase;
    u8* headWork;
    s32 one;
    s32 i;

    i = 3;
    p = D_801E31E0 + 3;
    stepBase = D_8014403E;
    do {
        step = stepBase[0];
        count = D_80144199_BYTE;
        do {
            headWork = D_1F800044;
        } while (0);
    } while (0);
    one = 1;
    D_8014419E = one;
    do {
        step++;
        do {
            count++;
        } while (0);
    } while (0);
    stepBase[0] = step;
    D_80144199_BYTE = count;
    headWork[6] = one;
    for (; i >= 0; i--) {
        *p-- = 0;
    }
    D_1F800044[7] = 0;
    D_1F800044[0x0B] = 0;
    func_8014D8D4(3);
    work = D_1F800044;
    work[3]++;
}
