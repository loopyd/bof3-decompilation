#include "internal.h"

/* @source 0x801E47E0
 * @behavior Advances local-work fields +0x40/+0x44 by +0x0C and then +0x0C by
 * +0x18 while signed +0x40 is at most 0x13FFF; otherwise sets byte +0x03 to 2.
 */
void integrateMotionOrSet2(void) {
    Battle03LocalWork* work;

    work = (Battle03LocalWork*)D_1F800044;
    if ((s32)work->unk_40 <= 0x13FFF) {
        work->unk_40 += work->unk_0c;
        work->unk_44 += work->unk_0c;
        work->unk_0c += work->unk_18;
    } else {
        work->unk_03 = 2;
    }
}
