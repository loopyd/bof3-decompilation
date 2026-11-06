#include "bof3/ui/shop00_internal.h"

/* @source 0x801E4DF4
 * @behavior shop panel emitter reached from the panel state dispatcher
 *           func_801E4338, which forwards the global panel task root: emits the
 *           0x9A by 0x39 shaded panel rectangle through func_801AE3F0 at the
 *           task's x/field-6 halfwords offset by 3 with the constant 0 and the
 *           main-RAM CLUT-bank byte D_80144952 as the two trailing stack
 *           arguments, draws the panel frame through func_801DD350 at the same
 *           task halfwords with the constants 0x12 and 6, and submits the
 *           texture row selected by the task byte at 0xA offset by 0x100 through
 *           func_8014F800 at (x + 0xE, field-6 + 4) with the constants 0 and
 *           0xFF and the word returned by func_801502D0 as the fifth argument.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801E4DF4(PanelTask* task) {
  u32 texture;

  func_801AE3F0(task->x + 3, task->field_06 + 3, 0x9A, 0x39, 0, D_80144952);
  func_801DD350(task->x, task->field_06, 0x12, 6);
  texture = func_801502D0(*((u8*)task + 0xA) | 0x100);
  func_8014F800(task->x + 0xE, task->field_06 + 4, 0, 0xFF, texture);
}
