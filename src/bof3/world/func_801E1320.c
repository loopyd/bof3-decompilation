#include "bof3/world/area03004_internal.h"

/* @source 0x801E1320
 * @behavior draws a value-dependent icon row and formatted numeric label.
 * @status partial
 * @match 87.50
 * @residual stack-frame size and copied-blob load scheduling differ
 */
void func_801E1320(s16 x, s16 y)
{
    IconData icons = D_801D0C3C;
    ThresholdData thresholds = D_801D0C70;
    u32 value;
    u32 index;
    IconRecord* record;
    u8* text;

    submitTpageDrawMode(3, 1);
    func_801E0DCC(6, 1, x, y);
    func_801E0DCC(7, 1, (s16)(x + 0x58), y);

    value = func_801E0B6C();
    index = 0;
    while ((value & 0xFFFF) >= thresholds.thresholds[index]) {
        index++;
    }

    record = &icons.records[index];
    D_801454EC = index;
    func_801E0DCC(record->icon, 1, (s16)(x + 0x1D), (s16)(y + 0x1D));
    if (record->first != 0) {
        func_801E0DCC(0x48, 1, (s16)(x + record->offset + 0x1D), (s16)(y + 0x21));
        if (record->second != 0) {
            func_801E0DCC(0x48, 1, (s16)(x + record->offset + 0x24), (s16)(y + 0x21));
        }
    }

    sprintf((char*)D_80145AD4, (char*)D_801D0C8C, value & 0xFFFF);
    text = D_80145AD4;
    while (text < D_80145AD4 + 5) {
        if (*text == 0x20) {
            *text = 0xFF;
        }
        text++;
    }
    func_8014F800((s16)(x + 0x45), (s16)(y + 9), 0, 5, (u32)D_80145AD4);

}
