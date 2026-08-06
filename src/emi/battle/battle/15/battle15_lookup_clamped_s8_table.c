#include "internal.h"

/* @source 0x800A7648
 * @behavior Adds 2 to signed index, clamps it to [0,4], then returns D_800B4E8C[index].
 */
s8 battle15_lookup_clamped_s8_table(s8 index) {
    index += 2;
    if (index < 0) {
        index = 0;
    }
    if (index >= 5) {
        index = 4;
    }
    return D_800B4E8C[index];
}
