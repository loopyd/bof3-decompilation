#include "bof3/world/area03004_internal.h"

/* @source 0x801DAF18
 * @behavior Advance the shared selection counters, clear D_801440B1 and publish the
 * selected record's y word to D_8014930C. Call func_8015D4F8(0, 0, 100, 8),
 * select shared mode 7 and advance work byte 3.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * The field-address temporary is load-bearing: taking the address of y_38
 * before the counter store keeps the 0x801E3208 index load and the 0x98 index
 * chain in the head block, so the D_80144199_BYTE load/increment/store stay at
 * their original positions. Folding the subscript into the store statement
 * instead lets the -O2 scheduler sink the counter trio behind the index chain
 * and reuse v1 for it.
 */
void publishSelectedRecordYAndAdvanceWorkStepWorld00Area03004(void) {
    u8 sharedCount;
    u8* work;
    s32* selectedY;

    D_8014419E = 2;
    sharedCount = D_80144199_BYTE;
    D_801440B1 = 0;
    selectedY = &D_80146888[D_801E3208].y_38;
    D_80144199_BYTE = sharedCount + 1;
    D_8014930C = *selectedY;
    func_8015D4F8(0, 0, 100, 8);
    work = D_1F800044;
    modeByte = 7;
    work[3]++;
}
