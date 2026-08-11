#include "bof3/world/area03004_internal.h"

/* @behavior initializes the shared motion state and advances the scratch
 * work-record action byte when the area gate flag is clear.
 * @source 0x801DAE3C
 * @status partial
 * @match 83.33
 * @residual Same-size allocator/scheduling mismatch: original keeps 0x40 in
 * v0 across the scratch-pointer load; current uses v1 and stores before it.
 */
void func_801DAE3C(void) {
    u8* work;
    s32 value = 0x40;

    if (D_80149332 == 0) {
        work = D_1F800044;
        D_8014932E = value;
        D_8014930C = 0x3C0000;
        work[3]++;
    }
}
