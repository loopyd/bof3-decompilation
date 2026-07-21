#include "internal.h"

/* @source 0x801E25CC
 * @behavior Decrements a frame counter; when it wraps to zero, increments
 *           a secondary counter.
 */
void func_801E25CC(void) {
    volatile u8 *p = &D_80148654;
    u8 val = *p - 1;
    *p = val;
    if (val == 0) {
        D_80148652 += 1;
    }
}
