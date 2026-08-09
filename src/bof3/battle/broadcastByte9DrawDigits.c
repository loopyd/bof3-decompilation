#include "bof3/battle/battle03_internal.h"

/* @source 0x801E6A54
 * @behavior Copies byte +0x09 from the scratchpad-selected object to scratchpad bytes 0x00 through 0x02, then calls func_801D99AC(0, 0, 0xB).
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void broadcastByte9DrawDigits(void)
{
    u8 value;

    value = SPAD_PTR_SLOT(u8, 0x44)[9];
    SPAD_REF(u8, 2) = value;
    SPAD_REF(u8, 1) = value;
    SPAD_REF(u8, 0) = value;
    func_801D99AC(0, 0, 0xB);
}
