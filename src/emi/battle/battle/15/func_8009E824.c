#include "internal.h"

/* @behavior UNKNOWN: exact behavior is not yet documented. */

/* @calls resetSelectionApplyInput with argument 0x20
 * @source 0x8009E824
 */
void func_8009E824(void) {
  resetSelectionApplyInput(0x20);
}
