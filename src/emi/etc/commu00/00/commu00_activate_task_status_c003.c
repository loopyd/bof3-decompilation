#include "internal.h"

/* @source 0x801F0EC8
 * @behavior marks the task's scratchpad record active and writes its 0xC003 status word.
 */
extern u16 commu00_task_label_words[1];

void commu00_activate_task_status_c003(u8 task_index) {
  volatile s8* ptr = (volatile s8*)(*((void**)0x1F800044));
  ptr[6] = 1;
  commu00_task_label_words[(task_index & 0xFF) * 0x98 / 2] = 0xC003;
}
