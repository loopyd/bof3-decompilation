#include "bof3/battle/battle03_internal.h"

/* @source 0x801DE754
 * @behavior Consumes the battle dispatch slot pointer published at 0x801EB4DC:
 * ORs the slot byte +0x01 into the dispatched-handler mask at 0x80146329,
 * decrements the slot countdown halfword +0x08 unless it already reads 0xFF,
 * then copies scratchpad byte 0x00 (the slot index) to the UI byte at 0x8014833A
 * while the refreshed countdown is non-zero; once the countdown reaches zero it
 * clears the slot byte +0x00 and, when func_801DE858 reports no queued kind-4
 * slot, sets the UI byte at 0x80148333 to 2.
 * @status exact
 * @match 100.00
 * @residual none
 * Live audit: 42/42 instructions, 168 bytes, live byte match.
 */
void func_801DE754(void) {
  volatile u8* slot;
  volatile u8* next;
  volatile u8* mask;
  u16          half;

  slot = D_801EB4DC;
  mask = &D_80146329;
  *mask |= slot[1];

  half = FIELD_REF(u16, slot, 8);
  if (half != 0xFFu) {
    FIELD_REF(u16, slot, 8) = half - 1u;
  }

  next = D_801EB4DC;
  half = FIELD_REF(u16, next, 8);
  if (half == 0u) {
    next[0] = 0;
    if (func_801DE858(4) != 0u) {
      *(u8*)0x80148333u = 2;
    }
  } else {
    *(u8*)0x8014833Au = SPAD_REF(u8, 0);
  }
}
