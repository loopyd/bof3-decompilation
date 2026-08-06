#include "internal.h"

/* @source 0x8009E200
 * @behavior initializes the selected state record fields
 */
void setRecordFlag8Field6Neg20(void)
{
    *((volatile u8*)D_801463A0 + 8) = 2;
    (*(s16*)((u32)D_801463A0 + 6)) = -0x14;
}
