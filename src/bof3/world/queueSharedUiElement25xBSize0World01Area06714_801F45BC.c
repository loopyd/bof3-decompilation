#include "bof3/bof3.h"

void func_801A4BC0(s16 x, s16 y, u32 size);

/* @source 0x801F45BC
 * @behavior Queues the shared UI element at (0x25, 0xB) with size 0 through
 * the shared marker queue func_801A4BC0; takes no arguments and returns
 * nothing, and its 0x18-byte frame only keeps $ra across the call.
 * @status exact
 * @match 100.00
 * @residual none
 */
void queueSharedUiElement25xBSize0World01Area06714_801F45BC(void) {
  func_801A4BC0(0x25, 0xB, 0);
}
