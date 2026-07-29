#include "internal.h"

/* @source 0x801EA848
 * @behavior Advances the active panel state and raises its update flag when two gate bytes differ.
 */
void func_801EA848(void) {
    if (D_801EC328 != D_801EBF04) {
        ((u8*)D_80148648)[3]++;
        D_801EC2E4 = 1;
    }
}
