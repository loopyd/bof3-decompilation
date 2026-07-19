#include "internal.h"

extern u8 D_80144F5A[];

/* @source 0x801BDB7C
 * @behavior scans 3 bytes in a mode-indexed table at D_80144F5A for 0xFF;
 *            returns index of first match (0-2), or 3 if none.
 */
u8 func_801BDB7C(u8 mode) {
    register u8 sentinel asm("a2") = 0xFF;
    s32 index = 0;
    u8 masked = mode & sentinel;
    u8 *ptr = D_80144F5A + masked * 3;

    while (index < 3) {
        if (*ptr == sentinel) {
            return index;
        }
        index++;
        ptr++;
    }

    return 3;
}
