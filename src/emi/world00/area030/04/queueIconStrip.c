#include "internal.h"

/* @behavior queues the two AREA030 icon strip primitives for the selected mode.
 * @source 0x801D2034
 */
void queueIconStrip(s16 arg0, s16 arg1, u8 arg2, s8 arg3) {
  submitTpageDrawMode(0, arg2);
  func_801E0DCC(0x17, arg2, arg0, arg1);
  func_801E0DCC((u8)(arg3 + 0x17), arg2, (s16)(arg0 + 0x28), arg1);
}
