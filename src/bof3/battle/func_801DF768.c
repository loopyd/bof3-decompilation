#include "bof3/battle/battle03_internal.h"

/* @source 0x801DF768
 * @behavior Index 0 of the local byte-2 handler table D_801EB154 dispatched by
 * func_801DF3F8. While the battle selection gate byte 0x801462EF is set and the
 * active selection slot byte D_801EB4D8 either equals scratch work byte +0x05 or
 * carries bit 0x80, submits the scratch work record's own positional effect
 * through func_8019651C (zero offsets, mode 0), stores the returned effect byte
 * at scratch work byte +0x07 (the array index the next handler func_801DF7EC
 * expands into the 12-byte D_80145BD4 records) and advances the scratch state
 * byte +0x02 to 1.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801DF768(void) {
  if (D_801462EF != 0) {
    if (*D_801EB4D8 == battleWork[5] || (*D_801EB4D8 & 0x80) != 0) {
      battleWork[7] = func_8019651C((void*)battleWork, 0, 0, 0, 0);
      battleWork[2] = 1;
    }
  }
}
