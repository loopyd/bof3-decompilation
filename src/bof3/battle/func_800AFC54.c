#include "bof3/battle/battle15_internal.h"

/* @behavior Copies 0x100 halfwords between fixed battle RAM buffers and sets the completion flag.
 * @source 0x800AFC54
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void func_800AFC54(void) {
  u32        i;
  u16*       dst;
  const u16* src;

  i = 0u;
  dst = PSX_PTR(u16, 0x80037C00u);
  src = PSX_PTR(const u16, 0x80033C00u);
  do {
    *dst++ = *src++;
    i++;
  } while (i < 0x100u);
  PSX_REF(u8, 0x80145988u) = 1u;
}
