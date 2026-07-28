#include "internal.h"

/* @source 0x800A52C4
 * @behavior dispatches the byte-selected battle handler from D_800B4D00.
 */
void func_800A52C4(void) {
  D_800B4D00[D_801462E4]();
}
