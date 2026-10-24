#include "bof3/battle/battle15_internal.h"

/* @source 0x8009E200
 * @behavior initializes the selected state record fields
 * @status exact
 * @match 100.00
 * @residual none
 */
void setRecordFlag8Field6Neg20(void)
{
    *((volatile u8*)D_8014639C.record + 8) = 2;
    (*(s16*)((u32)D_8014639C.record + 6)) = -0x14;
}
