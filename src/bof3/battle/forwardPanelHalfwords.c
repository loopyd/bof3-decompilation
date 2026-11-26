#include "bof3/battle/battle03_internal.h"

/* @source 0x801E91CC
 * @behavior Reads two signed panel halfwords and calls
 * drawBattlerStatusColumns.
 * @status exact
 * @match 100.00
 * @residual none
 */
void forwardPanelHalfwords(void) {
    drawBattlerStatusColumns(*(s16 *)&((u8*)panelTaskRoot)[4], *(s16 *)&((u8*)panelTaskRoot)[6]);
}
