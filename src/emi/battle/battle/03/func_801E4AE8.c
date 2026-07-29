#include "internal.h"

/* @source 0x801E4AE8
 * @behavior loads the non-volatile scratchpad pointer cell at 0x1F800044,
 * then invokes the byte-2-selected handler from the local 0x801EB460 table.
 */
void func_801E4AE8(void) {
  D_801EB460[g_battle03_work[2]]();
}
