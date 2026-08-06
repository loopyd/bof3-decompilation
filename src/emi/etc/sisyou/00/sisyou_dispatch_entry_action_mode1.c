#include "internal.h"

/* @source 0x801D3350
 * @behavior dispatches the selected entry's +0x11 action.
 */
void sisyou_dispatch_entry_action_mode1(void) {
  func_801D10AC((u16)(sisyou_master_action_base_table[sisyou_master_index] + 0x11));
}
