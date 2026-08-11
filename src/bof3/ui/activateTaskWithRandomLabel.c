#include "bof3/ui/commu00_internal.h"
#include <stdlib.h>

/* @source 0x801F0BF4
 * @behavior activates a task slot and assigns it a small randomized label
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void activateTaskWithRandomLabel(u8 task_index) {
  u8 value;
  u8 narrow_task_index;
  u32 offset;

  narrow_task_index = task_index & 0xFF;
  value = rand() & 7;
  if (value >= 5) {
    value -= 4;
  }

  offset = narrow_task_index * 0x98;
  D_8014688E[offset] = 1;
  taskLabelWords[offset / 2] = value + 0xC2;
}
