#include "bof3/bof3.h"

void func_801A4BC0(s16 x, s16 y, u32 size);

/* @source 0x801F4D58
 * @behavior Queues the shared world-marker element at (0x9, 0x3) with size
 * 0x50 through the shared marker queue func_801A4BC0; takes no arguments and
 * returns nothing, and its 0x18-byte frame only keeps $ra across the call.
 * @status exact
 * @match 100.00
 * @residual none
 */
void queueSharedUiElement9x3Size50World04Area17213_801F4D58(void) {
  func_801A4BC0(0x9, 0x3, 0x50);
}
