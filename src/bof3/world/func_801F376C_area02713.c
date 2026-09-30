#include "bof3/world/area02713_internal.h"

/* @source 0x801F376C
 * @behavior queues the six shared UI elements of a 2x3 marker block through
 *           func_801A4BC0: the element at (0x27, 0x25) with size 0, the element
 *           at (0x27, 0x26) with size 0, the element at (0x27, 0x27) with size
 *           0, the element at (0x28, 0x25) with size 0, the element at
 *           (0x28, 0x26) with size 0 and the element at (0x28, 0x27) with size
 *           0.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801F376C(void) {
  func_801A4BC0(0x27, 0x25, 0);
  func_801A4BC0(0x27, 0x26, 0);
  func_801A4BC0(0x27, 0x27, 0);
  func_801A4BC0(0x28, 0x25, 0);
  func_801A4BC0(0x28, 0x26, 0);
  func_801A4BC0(0x28, 0x27, 0);
}
