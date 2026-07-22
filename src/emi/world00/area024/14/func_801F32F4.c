#include "internal.h"

/* @behavior increments the byte at offset 1 of the current work pointer.
 * @source 0x801F32F4
 */
void func_801F32F4(void) {
  u8* work;

  work = WORLD00_AREA024_WORK_PTR;
  work[1]++;
}
