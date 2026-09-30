#include "bof3/battle/battle03_internal.h"

/* @source 0x801E69C0
 * @behavior Copies byte +0x09 of the scratchpad-selected object into scratchpad
 * bytes 0x00 through 0x02, calls func_801D99AC(0, 0, 0xB), then increments that
 * object's byte +0x09 and, when the incremented value is 0xFF, increments its
 * byte +0x01.
 * @status exact
 * @match 100.00
 * @residual none
 */
void broadcastByte9DrawDigitsAndAdvance(void)
{
    u8 value;
    u8 next;

    value = D_1F800044->pad_09[0];
    SPAD_REF(u8, 2) = value;
    SPAD_REF(u8, 1) = value;
    SPAD_REF(u8, 0) = value;
    func_801D99AC(0, 0, 0xB);

    next = D_1F800044->pad_09[0];
    next++;
    D_1F800044->pad_09[0] = next;
    if (next == 0xFF) {
        D_1F800044->unk_01 = (u8)(D_1F800044->unk_01 + 1);
    }
}
