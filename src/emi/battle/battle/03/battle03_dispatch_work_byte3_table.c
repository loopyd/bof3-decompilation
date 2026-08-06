#include "internal.h"

/* @source 0x801DFC70
 * @behavior loads the non-volatile scratchpad pointer cell at 0x44, uses byte 3
 * of its pointed-to local work as an index, and calls that handler-table entry.
 */
void battle03_dispatch_work_byte3_table(void) {
  D_801EB1D4[g_battle03_work[3]]();
}
