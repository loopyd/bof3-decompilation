#include "internal.h"

/* @behavior sets the shared flag D_801490A8 to -1 and, while the gate
 * byte D_801490C7 is zero, writes 10, 5, and 0xFF to D_801448EB,
 * D_801448EC, and D_801448ED.
 * @source 0x801F36A0
 */
void func_801F36A0(void)
{
    D_801490A8 = 0xFFFF;
    if (D_801490C7 == 0) {
        D_801448EB = 10;
        D_801448EC = 5;
        D_801448ED = 0xFF;
    }
}
