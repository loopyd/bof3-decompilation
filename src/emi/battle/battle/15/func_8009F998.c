#include "internal.h"

/* @behavior UNKNOWN: exact behavior is not yet documented. */

/* @calls battle15_reset_selection_apply_input with argument 0x80
 * @source 0x8009F998
 */
void func_8009F998(void) {
  battle15_reset_selection_apply_input(0x80);
}
