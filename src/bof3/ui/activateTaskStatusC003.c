#include "bof3/ui/commu00_internal.h"

extern u16 taskLabelWords[1];

/* @source 0x801F0EC8
 * @behavior Stores 1 at byte +6 of the scratchpad work pointer, then writes
 * 0xC003 to the taskLabelWords row selected by task_index with a 0x98-byte
 * stride.
 * @status exact
 * @match 100.00
 * @residual none
 */
void activateTaskStatusC003(u8 task_index) {
  volatile s8* ptr = (volatile s8*)(*((void**)0x1F800044));
  ptr[6] = 1;
  taskLabelWords[(task_index & 0xFF) * 0x98 / 2] = 0xC003;
}
