#include "internal.h"

/* @source 0x801D3258
 * @behavior starts the selected entry's +4 action unless the local state is already two.
 */
void sisyou_start_entry_action_mode0(void) {
  u8* state = &D_80143BB0;

  if (*state == 2) {
    return;
  }
  func_80150224((s16)(sisyou_master_action_base_table[sisyou_master_index] + 4));
  *state = 2;
  sisyou_mode_index = 6;
}
