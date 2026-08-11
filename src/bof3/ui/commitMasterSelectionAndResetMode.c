#include "bof3/ui/sisyou00_internal.h"

/**
 * Reset the SISYOU mode and advance the shared selection counter, optionally
 * copying the current master's selection into the active record.
 *
 * @source 0x801D3ED0
 * @behavior If the global state is not 2, copy the selected master's byte
 * into the active 152-byte record unless it is 0xFF, reset modeIndex, and
 * increment the shared selection counter.
 */
void commitMasterSelectionAndResetMode(void)
{
    u8 value;
    u8 recordIndex;

    if (D_80143BB0 != 2) {
        value = D_801D4270[masterIndex];
        if (value != 0xFF) {
            recordIndex = D_80146867;
            D_80146889[recordIndex * 152] = value;
        }
        modeIndex = 0;
        D_801448EC++;
    }
}
