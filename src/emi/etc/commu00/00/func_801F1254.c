#include "internal.h"

/* D_80146904: u16 array, indexed by arg * 76 (byte offset / sizeof(u16)) */
extern u16 D_80146904[1];

/* @source 0x801F1254
 * @behavior sets scratchpad slot 6 and writes 0xC00A to task-dependent u16 entry in D_80146904
 */
void func_801F1254(u8 task_index) {
  volatile s8* ptr = (volatile s8*)(*((void**)0x1F800044));
  ptr[6] = 1;
  D_80146904[(task_index & 0xFF) * 76] = 0xC00A;
}
