#include "internal.h"

/* @behavior calls the entry-1 update slice, then finalizes the shared front-end frame.
 * @source 0x801993F0
 */
void game_front_finalize_frame(void) {
  func_801D0D9C();
  func_80158C80();
}
