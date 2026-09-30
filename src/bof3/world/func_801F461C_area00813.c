#include "bof3/world/area00813_internal.h"

/* @source 0x801F461C
 * @behavior queues four shared UI elements through func_801A4BC0: the element
 *           at (0x55, 0x1C) with size 0, the element at (0x55, 0x1D) with size
 *           0, the element at (0x54, 0x1C) with size 0 and the element at
 *           (0x54, 0x1D) with size 0.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801F461C(void) {
  func_801A4BC0(0x55, 0x1C, 0);
  func_801A4BC0(0x55, 0x1D, 0);
  func_801A4BC0(0x54, 0x1C, 0);
  func_801A4BC0(0x54, 0x1D, 0);
}
