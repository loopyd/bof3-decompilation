#include "bof3/world/area03004_internal.h"

/* @source 0x801E1320
 * @behavior draws the AREA030 value row at the requested origin: selects
 * graphics mode 3/1, draws sprites 6 and 7 at (x, y) and (x + 0x58, y), reads
 * the func_801E0B6C summary value, scans the 13 halfword thresholds of the
 * frame copy of D_801D0C70 for the first entry above that value's low 16 bits,
 * publishes the resulting index into D_801454EC and draws the icon record of
 * the matching frame copy of D_801D0C3C at (x + 0x1D, y + 0x1D) followed by
 * sprite 0x48 at the record's offset-derived x positions on the y + 0x21 row
 * for its two flag bytes, then formats that value through D_801D0C8C into
 * D_80145AD4, rewrites the first five spaces to 0xFF and submits the result at
 * (x + 0x45, y + 9) through func_8014F800.
 * @status partial
 * @match 90.40
 * @residual the threshold scan loop is rotated by the loop optimizer: it peels
 * the index-0 test and closes with a conditional back edge, so the counter lands
 * in $v1 and the scan base is re-derived from $sp per iteration where the
 * original tests at the top, jumps back unconditionally with the counter in $a0
 * and reuses the $a3 icon base for both the scan and the following record index;
 * the record selection, D_801454EC store and the trailing space fixup then sit
 * in a shifted schedule and the object stays twelve bytes longer. Array
 * subscripting, pointer-arithmetic subscripting and a break-out form all emit
 * that same rotated loop, so the shape is a loop-optimizer decision rather than
 * an address-spelling one
 */
void func_801E1320(s16 x, s16 y)
{
    IconData icons = D_801D0C3C;
    ThresholdData thresholds = D_801D0C70;
    u32 value;
    u32 index;
    IconRecord* record;
    s32 i;

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
        func_801E0DCC(0x48, 1, (s16)(record->offset + 0x1D + x), (s16)(y + 0x21));
        if (record->second != 0) {
            func_801E0DCC(0x48, 1, (s16)(record->offset + x + 0x24), (s16)(y + 0x21));
        }
    }

    sprintf((char*)D_80145AD4, (char*)D_801D0C8C, value & 0xFFFF);
    for (i = 0; i < 5; i++) {
        if (D_80145AD4[i] == 0x20) {
            D_80145AD4[i] = 0xFF;
        }
    }
    func_8014F800((s16)(x + 0x45), (s16)(y + 9), 0, 5, (u32)D_80145AD4);
}
