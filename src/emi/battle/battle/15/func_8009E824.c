#include "internal.h"

/* @behavior UNKNOWN: exact behavior is not yet documented. */

/* @calls battle15_reset_selection_apply_input with argument 0x20
 * @source 0x8009E824
 */
void func_8009E824(void) {
  battle15_reset_selection_apply_input(0x20);
}
