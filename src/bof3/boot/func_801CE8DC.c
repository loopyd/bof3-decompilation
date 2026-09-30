#include "bof3/boot/logo_internal.h"

/*
 * @source 0x801CE8DC
 * @behavior publishes the LOGO display environment selected by the double
 * buffer index D_801EB4A8 through PutDispEnv, then flips the index for the next
 * frame.
 * @status exact
 * @match 100.00
 * @residual none
 * live comparison is instruction- and byte-exact.
 */
void func_801CE8DC(void) {
  s32 index;

  index = D_801EB4A8;
  PutDispEnv(&D_801EB480[index]);
  D_801EB4A8 = D_801EB4A8 ^ 1;
}
