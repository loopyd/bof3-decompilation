#include "bof3/world/area03004_internal.h"

/* @behavior draws the AREA030 panel combination for the shared 0x44/0x4E
 * origin through func_801E19CC; when the shared pad state word D_80145AA8
 * shares a bit with the pressed-button masks D_80145AC2 / D_80145AC4 it
 * advances byte 3 of the work record published at the scratchpad cursor
 * 0x1F800044; then appends the dim tile through appendDimTile.
 * @source 0x801DA498
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801DA498(void) {
    func_801E19CC(0x44, 0x4E);
    if (D_80145AA8 & (D_80145AC2 | D_80145AC4)) {
        u8 *work = D_1F800044;
        work[3]++;
    }
    appendDimTile();
}
