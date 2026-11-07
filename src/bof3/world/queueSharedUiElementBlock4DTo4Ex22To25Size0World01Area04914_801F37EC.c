#include "bof3/bof3.h"

void func_801A4BC0(s16 x, s16 y, u32 size);

/* @source 0x801F37EC
 * @behavior Queues the eight shared UI elements of one two-by-four block
 *           through the shared marker queue 0x801A4BC0: the four elements at
 *           y = 0x22..0x25 of x = 0x4D followed by the four elements at
 *           y = 0x22..0x25 of x = 0x4E, every one with size 0. Takes no
 *           arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void queueSharedUiElementBlock4DTo4Ex22To25Size0World01Area04914_801F37EC(void) {
  func_801A4BC0(0x4d, 0x22, 0);
  func_801A4BC0(0x4d, 0x23, 0);
  func_801A4BC0(0x4d, 0x24, 0);
  func_801A4BC0(0x4d, 0x25, 0);
  func_801A4BC0(0x4e, 0x22, 0);
  func_801A4BC0(0x4e, 0x23, 0);
  func_801A4BC0(0x4e, 0x24, 0);
  func_801A4BC0(0x4e, 0x25, 0);
}
