#include "bof3/ui/sisyou00_internal.h"

/* @source 0x801D38A8
 * @behavior starts the selected entry's +4 action unless the local state is already two.
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void startEntryActionMode1(void) {
  u8* state = &D_80143BB0;

  if (*state == 2) {
    return;
  }
  func_80150224((s16)(masterActionBaseTable[masterIndex] + 4));
  *state = 2;
  modeIndex = 6;
}
