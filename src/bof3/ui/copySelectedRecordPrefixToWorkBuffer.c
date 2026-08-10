#include "bof3/ui/sisyou00_internal.h"

/* @source 0x801D39CC @behavior Copies five bytes from the selected 164-byte
 * main-RAM record into a shared work buffer, then clears its terminator.
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void copySelectedRecordPrefixToWorkBuffer(u8 record_index)
{
    u8* source;
    s32 i;

    source = D_80144968 + ((u32)record_index * 164);
    for (i = 0; i < 5; i++) {
        D_801490D8[i] = source[i];
    }
    D_801490D8[5] = 0;
}
