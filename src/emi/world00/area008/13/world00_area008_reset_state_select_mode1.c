#include "internal.h"

/* @behavior Clears scratch byte 9 and local state, then enables scratch byte 1.
 * @source 0x801F2C5C
 */
void world00_area008_reset_state_select_mode1(void) {
  g_world00_area008_work->unk_09 = 0u;
  world00_area008_countdown = 0u;
  g_world00_area008_work->mode = 1u;
}
