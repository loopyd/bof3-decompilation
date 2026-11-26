#include "bof3/battle/battle03_internal.h"

/* @source 0x801E9910
 * @behavior Pushes the active panel halfwords +0x4/+0x6 and byte +0xA into
 * func_801D8AE4 and, while the battle selection gate byte 0x801462EF is set,
 * activates the panel task for the queued selection slot whose first byte
 * equals panel byte +0xA, then again when that slot byte has bit 0x40 set.
 * @status exact
 * @match 100.00
 * @residual none
 */
void renderPanelActivateTask(void) {
  drawBattlerStatusPanel(*(s16 *)&((u8*)panelTaskRoot)[4], *(s16 *)&((u8*)panelTaskRoot)[6],
                         ((u8*)panelTaskRoot)[0xA]);

  if (D_801462EF != 0) {
    if (*D_801EB4D8 == ((u8*)panelTaskRoot)[0xA]) {
      activatePanelTask();
    }
    if ((*D_801EB4D8 & 0x40) != 0) {
      activatePanelTask();
    }
  }
}
