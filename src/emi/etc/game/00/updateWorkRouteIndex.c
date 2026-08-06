#include "internal.h"

/* @behavior Conditionally updates the route_index_08 byte in the scratchpad
 * work area. If bit 0 is set the value is left unchanged. If the current
 * value matches arg_a or arg_b, the byte is set to arg_c; otherwise it is
 * set to arg_c ^ 4.
 * @source 0x801BB8E8
 */
void updateWorkRouteIndex(u8 arg_a, u8 arg_b, u8 arg_c) {
  struct GameWorkArea* work;
  u8                   current;

  work = SCRATCH_WORK;
  current = work->route_index_08;

  if (current & 1) {
    return;
  }

  if (current == arg_a) {
    work->route_index_08 = arg_c;
  } else if (current == arg_b) {
    work->route_index_08 = arg_c;
  } else {
    work->route_index_08 = arg_c ^ 4;
  }
}
