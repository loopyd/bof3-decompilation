#include "bof3/bof3.h"

void func_801F381C(s32 arg0);

/* @source 0x801F3534
 * @behavior Byte-2 state handler that both byte-2 dispatch tables of this
 *           overlay list at entry 2 (0x801F516C, indexed by the dispatcher at
 *           0x801F3014, and 0x801F5184, indexed by the dispatcher at
 *           0x801F3C10); either table indexes the work record published at the
 *           scratchpad pointer cell 0x1F800044 by its state byte at offset
 *           0x02. Subtracts 0x20 from that record's unsigned halfword at offset
 *           0x30, stores the truncated halfword, and, once the stored value
 *           read as a signed halfword falls below -0xFF, increments the record
 *           state byte at offset 0x02 so the next dispatch takes entry 3 of the
 *           same table (0x801F3590 on the 0x801F516C table, 0x801F3FD0 on the
 *           0x801F5184 table). It then calls the overlay record draw routine at
 *           0x801F381C with a zero mode argument, which takes that routine's
 *           zero branch; the callee reads the same halfword at offset 0x30 as a
 *           sprite screen offset. Takes no arguments and returns nothing; the
 *           record pointer is loaded before the frame is created, the 0x18-byte
 *           frame only keeps $ra at 0x10 across the call, and the slots after
 *           the halfword load and after the branch stay nops.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementHalfword30By20AndAdvanceWorkByte2BelowMinusFfThenDrawWorld03Area11813_801F3534(
    void) {
  u8* work;
  u16 half;

  work = SPAD_PTR_SLOT(u8, 0x44);
  half = *(u16*)(work + 0x30) - 0x20;
  *(u16*)(work + 0x30) = half;
  if ((s16)half < -0xFF) {
    work[2]++;
  }
  func_801F381C(0);
}
