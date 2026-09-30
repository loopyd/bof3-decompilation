#include "bof3/world/area00813_internal.h"

/* @source 0x801F4334
 * @behavior queues three shared UI elements through func_801A4BC0: the element
 *           at (0x33, 0x15) with size 0, the element at (0x33, 0x16) with size
 *           0 and the element at (0x32, 0x16) with size 0x51.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801F4334(void) {
  func_801A4BC0(0x33, 0x15, 0);
  func_801A4BC0(0x33, 0x16, 0);
  func_801A4BC0(0x32, 0x16, 0x51);
}
