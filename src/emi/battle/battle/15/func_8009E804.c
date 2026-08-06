#include "internal.h"

/* @behavior UNKNOWN: exact behavior is not yet documented. */

/* @calls resetSelectionApplyInput with argument 0x40
 * @source 0x8009E804
 */
void func_8009E804(void) {
  resetSelectionApplyInput(0x40);
}
