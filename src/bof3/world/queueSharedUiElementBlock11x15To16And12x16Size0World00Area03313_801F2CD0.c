#include "bof3/bof3.h"

/* @source 0x801A4BC0 @kind function */
extern void func_801A4BC0(s16 x, s16 y, u32 size);

/* @source 0x801F2CD0
 * @behavior Queues three shared UI elements through func_801A4BC0 at
 *           (0x11, 0x15), (0x11, 0x16) and (0x12, 0x16), all with size 0.
 *           Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Naming follows the shared UI queue siblings: the three original calls pass
 * the listed x/y coordinates and size zero. The shared callee declaration and
 * three-call statement shape follow the exact size-0x51 column sibling at
 * World03Area13613_801F2E08; this body uses its own original constants.
 */
void queueSharedUiElementBlock11x15To16And12x16Size0World00Area03313_801F2CD0(void) {
  func_801A4BC0(0x11, 0x15, 0);
  func_801A4BC0(0x11, 0x16, 0);
  func_801A4BC0(0x12, 0x16, 0);
}
