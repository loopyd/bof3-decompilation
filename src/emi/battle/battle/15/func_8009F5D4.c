#include "internal.h"

/* @behavior UNKNOWN: exact behavior is not yet documented. */

/* @calls resetSelectionApplyInput with argument 0x10
 * @source 0x8009F5D4
 */
void func_8009F5D4(void) {
  D_80146375 = 4;
  D_801463C0 = 0x57;
  resetSelectionApplyInput(0x10);
}
