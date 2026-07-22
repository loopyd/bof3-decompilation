#include "internal.h"

void func_8009DC6C(void) {
  /*
     * MATCHING_AID (user-approved register pin):
     * Pin half to $v1 so the sign-extension naturally flows v0→v1,
     * leaving $v0 free for the D_801463A0 pointer load. This matches
     * the original scheduling where lui+lw fills the addu→sra gap
     * and sh lands in the beqz delay slot.
     */
  register s32 half asm("v1");

  half = func_801DC044(D_80146374, D_80146394, 0xFFFF) / 2;
  ((u16*)D_801463A0)[2] = half;
  if (half != 0) {
    func_800A4238(0x20);
  }
}
