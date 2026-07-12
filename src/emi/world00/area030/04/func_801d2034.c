#include "internal.h"

/* @behavior queues the two AREA030 icon strip primitives for the selected mode.
 * @source 0x801d2034 FUN_801d2034
 */
void func_801d2034(s16 arg0, s16 arg1, u8 arg2, s8 arg3) {
  func_801e0c80(0, arg2);
  func_801e0dcc(0x17, arg2, arg0, arg1);
  func_801e0dcc((s16)(arg3 + 0x17), arg2, (s16)(arg0 + 0x28), arg1);
}
