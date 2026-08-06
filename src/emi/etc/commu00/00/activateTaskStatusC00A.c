#include "internal.h"

/* taskLabelWords: u16 array, indexed by arg * 76 (byte offset / sizeof(u16)) */
extern u16 taskLabelWords[1];

/* @source 0x801F1254
 * @behavior sets scratchpad slot 6 and writes 0xC00A to task-dependent u16 entry in taskLabelWords
 */
void activateTaskStatusC00A(u8 task_index) {
  volatile s8* ptr = (volatile s8*)(*((void**)0x1F800044));
  ptr[6] = 1;
  taskLabelWords[(task_index & 0xFF) * 76] = 0xC00A;
}
