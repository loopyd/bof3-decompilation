#include "bof3/battle/battle03_internal.h"

/* @behavior marks the current panel task active and advances its state byte.
 * @source 0x801EA174
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void activatePanelTask(void) {
  u8* volatile* root = (u8* volatile*)0x80150000u;
  u8*           temp_v1;

  root[-0x1e6e][0xf] = 1u;
  temp_v1 = root[-0x1e6e];
  temp_v1[3] = (u8)(temp_v1[3] + 1u);
}
