#include "bof3/world/area03004_internal.h"

/* @behavior decrements byte 9 of the work record published at the scratchpad
 * cursor 0x1F800044, redraws the AREA030 panel at the origin 0x44 + 0x50 *
 * remaining count through func_801E19CC, and once that count reaches zero arms
 * byte 6 of the record and advances its dispatch byte 3; the dim tile is
 * appended through appendDimTile last.
 * @source 0x801DA3F4
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801DA3F4(void) {
    u8 *work;

    work = D_1F800044;
    work[9] = work[9] - 1;
    func_801E19CC((s16)(D_1F800044[9] * 0x50 + 0x44), 0x4E);
    work = D_1F800044;
    if (work[9] == 0) {
        work[6] = 1;
        work = D_1F800044;
        work[3]++;
    }
    appendDimTile();
}
