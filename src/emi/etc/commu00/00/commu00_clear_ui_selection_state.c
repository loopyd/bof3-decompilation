#include "internal.h"

/* @source 0x801F1684
 * @behavior clears the three frontend UI selection-state bytes.
 */
void commu00_clear_ui_selection_state(void)
{
  func_8015C058();
  commu00_ui_mode = 0;
  commu00_fairy_progress[0] = 0;
  commu00_fairy_slot_index = 0;
}
