#include "bof3/bof3.h"

/* The shared state byte the SHISU overlay reads here. It lies below this
 * overlay's load address, in the main executable's work RAM, so the overlay
 * only reads it and no code of this payload writes it. The cell carries no
 * reviewed name in the target or shared map, so it stays address-anchored.
 */
extern u8 D_80144FF9; /* @source 0x80144FF9 @kind data */

/* No code of this payload reaches this span: it has no in-payload caller and no
 * word of the payload's code or script blob names it, so its caller and the
 * meaning of the returned code live outside the overlay and stay unknown here.
 */
/* @source 0x801D1E60
 * @behavior Maps the shared byte at 0x80144FF9 onto one of eight codes from 9
 *           through 16 by a descending unsigned range ladder: any value below
 *           0x26 returns 9, the bands 0x26 to 0x27, 0x28 to 0x2C, 0x2D to 0x31,
 *           0x32 to 0x3B, 0x3C to 0x40 and 0x41 to 0x43 return 10 through 15,
 *           and every other value returns 16. Each band is tested as an
 *           unsigned subtraction of its own base, so no lower-bound test is
 *           needed. The span takes no arguments, creates no frame, touches no
 *           memory beyond that load and leaves the code in $v0.
 * @status exact
 * @match 100.00
 * @residual none
 */
s32 mapByte80144FF9ToCodeShisu00_801D1E60(void)
{
    u32 value;

    value = D_80144FF9;
    if (value < 0x26) {
        return 9;
    }
    if (value - 0x26 < 2) {
        return 10;
    }
    if (value - 0x28 < 5) {
        return 11;
    }
    if (value - 0x2D < 5) {
        return 12;
    }
    if (value - 0x32 < 0xA) {
        return 13;
    }
    if (value - 0x3C < 5) {
        return 14;
    }
    if (value - 0x41 < 3) {
        return 15;
    }
    return 16;
}
