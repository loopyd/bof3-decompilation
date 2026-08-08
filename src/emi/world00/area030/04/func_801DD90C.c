#include "internal.h"

/* @behavior when the area gate is clear, advances its counter and initializes
 * bytes 2 and 3 of the current scratch work record.
 * @source 0x801DD90C
 */
void func_801DD90C(void) {
    u8* work;

    if (D_80144281 == 0) {
        D_8014403D++;
        work = D_1F800044;
        work[2] = 1;
        D_1F800044[3] = 0;
    }
}
