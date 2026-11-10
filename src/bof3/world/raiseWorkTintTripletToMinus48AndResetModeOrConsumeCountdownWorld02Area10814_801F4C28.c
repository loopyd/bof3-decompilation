#include "bof3/bof3.h"

extern u8* workRecord44World02Area10814; /* @source 0x1F800044 @kind data */
extern u8* D_80146884; /* @source 0x80146884 @kind data */
extern void func_80196670(u8 arg0);

/* @source 0x801F4C28
 * @behavior Entry 1 of the work-byte-4 handler table at 0x801F63C4.
 * Raises each signed tint byte at work offsets 0x5D through 0x5F by 1
 * while below -48. When the word at 0x5C masked by 0xFFFFFF00 equals
 * 0xD0D0D000, passes the global work record's effect byte at 0x93 to
 * shared helper 0x80196670, clears work flag bit 0x20, the mode byte
 * at 0x5C, the tint triplet and handler byte 4. Otherwise subtracts 2
 * from the global work record's countdown halfword at 0x7E. The work
 * pointer comes from scratchpad cell 0x1F800044; the global record
 * pointer comes from 0x80146884. Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void raiseWorkTintTripletToMinus48AndResetModeOrConsumeCountdownWorld02Area10814_801F4C28(void) {
  u8* record;
  u8* cursor;
  u8* work;

  if (((s8*)workRecord44World02Area10814)[0x5d] < -0x30) {
    ((s8*)workRecord44World02Area10814)[0x5d] += 1;
  }
  if (((s8*)workRecord44World02Area10814)[0x5e] < -0x30) {
    ((s8*)workRecord44World02Area10814)[0x5e] += 1;
  }
  if (((s8*)workRecord44World02Area10814)[0x5f] < -0x30) {
    ((s8*)workRecord44World02Area10814)[0x5f] += 1;
  }

  record = workRecord44World02Area10814;
  if ((*(u32*)(record + 0x5c) & 0xffffff00) == 0xd0d0d000) {
    func_80196670(D_80146884[0x93]);
    workRecord44World02Area10814[0] &= 0xdf;
    workRecord44World02Area10814[0x5c] = 0;
    cursor = workRecord44World02Area10814;
    cursor[0x5f] = 0;
    cursor[0x5e] = 0;
    cursor[0x5d] = 0;
    workRecord44World02Area10814[4] = 0;
  } else {
    work = D_80146884;
    *(u16*)(work + 0x7e) -= 2;
  }
}
