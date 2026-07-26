#include "internal.h"

/* @source 0x801F0EC8
 * @behavior marks the task's scratchpad record active and writes its 0xC003 status word.
 */
extern u16 D_80146904[1];

void func_801F0EC8(u8 task_index) {
  volatile s8* ptr = (volatile s8*)(*((void**)0x1F800044));
  ptr[6] = 1;
  D_80146904[(task_index & 0xFF) * 0x98 / 2] = 0xC003;
}
