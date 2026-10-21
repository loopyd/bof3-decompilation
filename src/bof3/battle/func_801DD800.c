#include "bof3/battle/battle03_internal.h"

/* @behavior Steps the oscillating factor byte 0x80146308 by two toward the
 * opposite bound: a value above 0x1E sets the direction latch 0x80146304 and a
 * zero value clears it, so the byte ramps up to 0x20 and back down to zero.
 * Its consumers func_801E38C0 and func_801DF7EC copy it into the effect record
 * bytes +0x02, +0x03 and +0x04 of the 12-byte D_80145BD4 records.
 * @source 0x801DD800
 * @status partial
 * @match 81.82
 * @residual allocation/scheduling: 18/22 instructions match at the same
 * 88-byte size. The original keeps the stepped result in $v0 and keeps an extra
 * copy of the loaded byte in $a1 for the falling arm (`move a1,a0` filling the
 * delay slot of the second test); this candidate allocates the result to $a1
 * with no copy, so that delay slot is empty and the two `addiu` plus the final
 * `sb` name $a1 instead of $v0/$a1.
 */
void func_801DD800(void) {
  u8* p;
  u8  value;
  u8  next;

  p = &BATTLE_GLOBAL_BYTE_6308;
  value = *p;
  if (value > 0x1Eu) {
    BATTLE_GLOBAL_BYTE_6304 = 1u;
  }
  if (value == 0u) {
    BATTLE_GLOBAL_BYTE_6304 = 0u;
  }
  next = value + 2u;
  if (BATTLE_GLOBAL_BYTE_6304 == 1u) {
    next = value - 2u;
  }
  *p = next;
}
