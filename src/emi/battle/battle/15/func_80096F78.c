#include "internal.h"

u32  func_801502D0(u32 arg0);
void func_801DE8C0(u8 arg0, u8 arg1, u32 arg2);

/* @source 0x80096F78
 * @behavior calls func_801502D0(0x4000), func_801DE8C0(2, 0xFF, result),
 *            then increments the byte counter at D_801462E4.
 */
void func_80096F78(void) {
    u32 temp;
    u8 *p;

    temp = func_801502D0(0x4000);
    func_801DE8C0(2, 0xFF, temp);
    p = (u8 *)&D_801462E4;
    *p = *p + 1;
}
