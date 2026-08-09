#include "bof3/ui/commu00_internal.h"

/* @source 0x801F0EC8
 * @behavior marks the task's scratchpad record active and writes its 0xC003 status word.
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
extern u16 taskLabelWords[1];

void activateTaskStatusC003(u8 task_index) {
  volatile s8* ptr = (volatile s8*)(*((void**)0x1F800044));
  ptr[6] = 1;
  taskLabelWords[(task_index & 0xFF) * 0x98 / 2] = 0xC003;
}
