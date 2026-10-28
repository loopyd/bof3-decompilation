#include "bof3/ui/game00_internal.h"

/*
 * @behavior Steps the scratch work sequence through func_8014DAEC and clears
 * D_80146250[0x12B] when its low return byte is nonzero. A nonzero result can
 * mean sequence end or a control redirect. The scratch work pointer and
 * D_80146250 need not identify the same record.
 * @source 0x801B2D54
 * @status exact
 * @match 100.00
 * @residual none
 */
void stepWorkSequenceAndClearByte12BOnNonzero(void)
{
    if (func_8014DAEC() != 0u) {
        D_80146250[0x12B] = 0;
    }
}
