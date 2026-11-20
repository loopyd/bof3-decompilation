#include "bof3/world/area03004_internal.h"

/* @source 0x801E1320
 * @behavior Copy the icon records and threshold halfwords, draw the header sprites, and
 * obtain the slot-value sum. Scan without a bound for the first unsigned
 * threshold above its low halfword, publish the index and draw the selected
 * icon. Draw the first optional sprite 0x48 when its flag is set; the second
 * requires both flags. Format the low halfword, replace the first five spaces
 * with 0xFF and draw five characters.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawSlotValueSummaryAndThresholdIconsWorld00Area03004(s16 x, s16 y)
{
    IconData icons = D_801D0C3C;
    ThresholdData thresholds = D_801D0C70;
    u32 slotValueTotal;
    u32 thresholdIndex;
    IconRecord* iconRecord;
    s32 characterIndex;

    submitTpageDrawMode(3, 1);
    appendIndexedSpriteWorld00Area03004(6, 1, x, y);
    appendIndexedSpriteWorld00Area03004(7, 1, (s16)(x + 0x58), y);

    slotValueTotal = sumValuesForNonzeroSlotAmountsWorld00Area03004();
    thresholdIndex = 0;
    while (1) {
        if ((slotValueTotal & 0xFFFF) < thresholds.thresholds[thresholdIndex]) {
            break;
        }
        thresholdIndex++;
    }

    iconRecord = &icons.records[thresholdIndex];
    D_801454EC = thresholdIndex;
    appendIndexedSpriteWorld00Area03004(iconRecord->icon, 1, (s16)(x + 0x1D), (s16)(y + 0x1D));
    if (iconRecord->first != 0) {
        appendIndexedSpriteWorld00Area03004(0x48, 1, (s16)(iconRecord->offset + 0x1D + x), (s16)(y + 0x21));
        if (iconRecord->second != 0) {
            appendIndexedSpriteWorld00Area03004(0x48, 1, (s16)(iconRecord->offset + x + 0x24), (s16)(y + 0x21));
        }
    }

    sprintf((char*)D_80145AD4, (char*)D_801D0C8C, slotValueTotal & 0xFFFF);
    for (characterIndex = 0; characterIndex < 5; characterIndex++) {
        if (D_80145AD4[characterIndex] == 0x20) {
            D_80145AD4[characterIndex] = 0xFF;
        }
    }
    func_8014F800((s16)(x + 0x45), (s16)(y + 9), 0, 5, (u32)D_80145AD4);
}
