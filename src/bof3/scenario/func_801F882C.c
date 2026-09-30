#include "bof3/scenario/scena00_internal.h"

/* @source 0x801F882C
 * @behavior Advances the scratchpad work object published at 0x1F800044 by one
 * fade step: it drops the object's halfword at offset 0x2E by 4, adds the
 * signed D_801FC9EC byte selected by the object's countdown byte at offset 0x09
 * shifted right by two to the halfword at offset 0x30, and when the object's
 * signed tint byte at offset 0x5D is nonzero it fades the three tint bytes at
 * offsets 0x5D/0x5F/0x5E by 2 each, advances the countdown byte at offset 0x09
 * and increments the transition byte at offset 0x06 once the low two bits of
 * that countdown byte are clear before running the following handler
 * func_801F8BCC; when the tint byte at offset 0x5D is already zero the step is
 * just the shared func_80196070 work-flag reset. Takes no arguments and returns
 * nothing; the address is entry 22 (word at 0x801FC9D8) of the per-frame
 * handler table at 0x801FC980 indexed by byte 0x01 of that work object.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801F882C(void) {
  u8* work;
  s8  fade;

  work = D_1F800044;
  *(u16*)(work + 0x2E) -= 4;
  *(u16*)(work + 0x30) += (s8)D_801FC9EC[D_1F800044[9] >> 2];
  fade = *(s8*)(work + 0x5D);

  if (fade != 0) {
    *(s8*)(work + 0x5D) = fade - 2;
    *(u8*)(D_1F800044 + 0x5F) -= 2;
    *(u8*)(D_1F800044 + 0x5E) -= 2;
    D_1F800044[9]++;

    if ((D_1F800044[9] & 3) == 0) {
      D_1F800044[6]++;
    }

    func_801F8BCC();
  } else {
    func_80196070();
  }
}
