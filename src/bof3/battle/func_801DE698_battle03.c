#include "bof3/battle/battle03_internal.h"

/* @source 0x801DE698
 * @behavior Consumes the battle dispatch slot pointer published at 0x801EB4DC.
 * While its countdown halfword +0x08 is not 0xFF it ORs the slot byte +0x01 into
 * the dispatched-handler mask at 0x80146329 and decrements the countdown; once
 * the countdown reaches zero it clears the slot byte +0x00, clears this slot bit
 * from the mask, and zeroes the UI gate at 0x801483C3 when func_801DE858 reports
 * a queued kind-1 slot. Every path that skips the zero-countdown work copies
 * scratchpad byte 0x00 to the UI byte at 0x801483CA.
 * @status exact
 * @match 100.00
 * @residual none
 * Live audit: 47/47 instructions, 188 bytes, live byte match.
 */
void func_801DE698(void) {
  volatile u8* slot;
  volatile u8* next;
  u8*          mask;
  u16          half;

  slot = D_801EB4DC;
  half = FIELD_REF(u16, slot, 8);
  if (half != 0xFFu) {
    mask = &D_80146329;
    *mask |= slot[1];
    half = FIELD_REF(u16, slot, 8) - 1u;
    FIELD_REF(u16, slot, 8) = half;
    if (half == 0u) {
      slot[0] = 0;
      next = D_801EB4DC;
      *mask &= (u8)~next[1];
      if (func_801DE858(1) != 0u) {
        D_801483C3 = 0;
      }
      return;
    }
  }
  *(u8*)0x801483CAu = SPAD_REF(u8, 0);
}
