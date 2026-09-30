#include "bof3/scenario/scena00_internal.h"

/* @source 0x801F8930
 * @behavior Advances the scratchpad work object published at 0x1F800044 by one
 * fade step: subtracts 2 from that object's halfword at offset 0x30 and, when
 * the object's signed tint byte at offset 0x5D is nonzero, fades the three tint
 * bytes at offsets 0x5D/0x5F/0x5E by 4 each, increments the object's
 * transition byte at offset 0x06 once the low three bits of its byte at offset
 * 0x09 are clear and runs the following handler func_801F8BCC; when that tint
 * byte is already zero the step is just the shared func_80196070 work-flag
 * reset. Takes no arguments and returns nothing; the address is entry 23 (word
 * at 0x801FC9DC) of the per-frame handler table at 0x801FC980 indexed by byte
 * 0x01 of that work object.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801F8930(void) {
  u8* work;
  s8  fade;

  work = D_1F800044;
  *(u16*)(work + 0x30) -= 2;
  fade = *(s8*)(work + 0x5D);

  if (fade != 0) {
    *(s8*)(work + 0x5D) = fade - 4;
    *(u8*)(D_1F800044 + 0x5F) -= 4;
    *(u8*)(D_1F800044 + 0x5E) -= 4;

    if ((D_1F800044[9] & 7) == 0) {
      D_1F800044[6]++;
    }

    func_801F8BCC();
  } else {
    func_80196070();
  }
}
