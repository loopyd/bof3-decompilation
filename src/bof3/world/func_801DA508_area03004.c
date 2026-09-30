#include "bof3/world/area03004_internal.h"

/* @behavior increments byte 9 of the work record published at the scratchpad
 * cursor 0x1F800044, redraws the AREA030 panel at the origin 0x44 - 0x50 *
 * that count through func_801E19CC, and once that count reaches 4 dispatches
 * sound cue 0x102 through func_8015DF18, arms byte 6 of the record and stores
 * 4 in its dispatch byte 3; the dim tile is appended through appendDimTile
 * last.
 * @source 0x801DA508
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801DA508(void) {
    u8 *work;

    work = D_1F800044;
    work[9] = work[9] + 1;
    func_801E19CC((s16)(0x44 - D_1F800044[9] * 0x50), 0x4E);
    if (D_1F800044[9] == 4) {
        func_8015DF18(0x102);
        D_1F800044[6] = 1;
        D_1F800044[3] = 4;
    }
    appendDimTile();
}
