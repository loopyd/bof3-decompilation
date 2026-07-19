#include "internal.h"

/* @source 0x800983C4
 * @behavior calls func_801502D0(0x4000), func_801DE8C0(2, 0xFF, result),
 *            then increments the byte counter at D_801462E4.
 */
void func_800983C4(void) {
    u32 temp;
    u8 *counter;

    counter = (u8 *)&D_801462E4;
    temp = func_801502D0(0x4000);
    func_801DE8C0(2, 0xFF, temp);
    *counter = *counter + 1;
}
