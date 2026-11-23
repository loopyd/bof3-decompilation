#include "bof3/world/area03004_internal.h"

/* @behavior Forms masked-coordinate differences from work words +0x34 and +0x38
 * using unsigned logical right shifts by three, stores the steps at
 * +0x0C and +0x10, seeds byte 9 with 8 and advances work mode byte 2.
 * @source 0x801DFFEC
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateMaskedCoordinateSteps(void)
{
    u8 *work;
    u8 *dispatchWork;
    s32 x;
    s32 y;

    work = D_1F800044;
    x = *(s32 *)(work + 0x34);
    work[9] = 8;
    y = *(s32 *)(work + 0x38);
    *(u32 *)(work + 0x0C) = (u32)((u32)((x & ~0x7FFF) - x) >> 3);
    dispatchWork = D_1F800044;
    *(u32 *)(work + 0x10) = (u32)((u32)((y & ~0x7FFF) - y) >> 3);
    dispatchWork[2] = (u8)(dispatchWork[2] + 1);
}
