#include "bof3/ui/shop00_internal.h"

/* Both the mode byte and the scaled record offset are byte-required locals:
 * computing them inline lets gcc reserve two extra frame words for the record
 * address, and the u8 local is what keeps the mode mask scheduled after the
 * first call. */
/* @source 0x801DB938
 * @behavior shop panel emitter: submits the shaded panel rectangle through
 *           func_801AE3F0 with the task's x and field-6 halfwords, the fixed
 *           0x8B by 0x9F size, the low byte of arg1 offset by 0xF0 and the
 *           main-RAM CLUT-bank byte D_80144952 as the trailing stack argument,
 *           then draws the same task twice through the shop row emitters
 *           func_801D95F4 and func_801D9254 with the leading byte of the
 *           0x140-byte-stride record D_80145FCC selected by task byte 0xC and
 *           the low byte of arg1.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801DB938(PanelTask* task, u32 arg1) {
  u32 off;
  u8 mode = arg1 & 0xFF;

  func_801AE3F0(task->x, task->field_06, 0x8B, 0x9F, (arg1 + 0xF0) & 0xFF,
                D_80144952);
  off = *((u8*)task + 0xC) * 0x140;
  func_801D95F4(task->x, task->field_06, D_80145FCC[off], mode);
  off = *((u8*)task + 0xC) * 0x140;
  func_801D9254(task->x, task->field_06, D_80145FCC[off], mode);
}
