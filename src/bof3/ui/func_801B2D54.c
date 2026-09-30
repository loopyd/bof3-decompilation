#include "bof3/ui/game00_internal.h"

/*
 * @behavior Runs the shared readiness helper func_8014DAEC and, when it
 * reports ready (non-zero byte), clears byte 0x12B of the shared work record
 * pointer D_80146250.
 * @source 0x801B2D54
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801B2D54(void)
{
    if (func_8014DAEC() != 0u) {
        D_80146250[0x12B] = 0;
    }
}
