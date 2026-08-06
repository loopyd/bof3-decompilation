#include "internal.h"

/* @behavior UNKNOWN: exact behavior is not yet documented. */

/* @calls resetSelectionApplyInput with argument 0x80
 * @source 0x8009F998
 */
void func_8009F998(void) {
  resetSelectionApplyInput(0x80);
}
