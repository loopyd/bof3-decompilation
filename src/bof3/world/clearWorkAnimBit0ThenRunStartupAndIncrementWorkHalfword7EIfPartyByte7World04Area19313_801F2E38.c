#include "bof3/bof3.h"

extern void* D_80146884; /* @source 0x80146884 @kind data */
/* @source 0x80145F09 @kind unknown - party byte (+0x79) of the 0x140-byte
 * stride local work records based at 0x80145E90, so this reads record 0. */
extern u8 D_80145F09;

void func_8015C088(void);

/* @source 0x801F2E38
 * @behavior Clears bit 0 of the byte at offset 0x74 of the work record
 *           published at 0x80146884 with an `andi 0xFE` read-modify-write, then,
 *           when the party byte of local work record 0 (0x80145F09, byte +0x79)
 *           holds 7, runs the shared front-end startup helper 0x8015C088 and
 *           increments that work record's unsigned halfword at offset 0x7E in
 *           place. The pointer cell 0x80146884 is re-read after the call, so the
 *           increment goes through the record published then rather than the one
 *           whose bit 0 was just cleared. Takes no arguments and returns
 *           nothing. The 0x18-byte frame keeps only $ra, because the record
 *           pointer is never held across the call; the 7 constant fills the load
 *           delay slot of the party-byte load ahead of the `bne` that skips
 *           straight to the shared epilogue.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearWorkAnimBit0ThenRunStartupAndIncrementWorkHalfword7EIfPartyByte7World04Area19313_801F2E38(
    void) {
  u8* record;

  record = D_80146884;
  record[0x74] &= ~1u;
  if (D_80145F09 == 7) {
    func_8015C088();
    record = D_80146884;
    *(u16*)(record + 0x7E) += 1;
  }
}
