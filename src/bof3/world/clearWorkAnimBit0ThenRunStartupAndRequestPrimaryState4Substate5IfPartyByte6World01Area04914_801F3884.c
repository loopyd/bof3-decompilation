#include "bof3/bof3.h"

/* @source 0x80145F09 @kind unknown - party byte (+0x79) of the 0x140-byte
 * stride local work records based at 0x80145E90, so this reads record 0. */
extern u8 D_80145F09;
extern void* D_80146884; /* @source 0x80146884 @kind data */
/* @source 0x80146874 @kind unknown */
extern s8 D_80146874;
/* @source 0x80146875 @kind unknown */
extern u8 D_80146875;

void func_8015C088(void);

/* @source 0x801F3884
 * @behavior Clears bit 0 of the byte at offset 0x74 of the work record
 *           published at 0x80146884 with an `andi 0xFE` read-modify-write. When
 *           the party byte of local work record 0 (0x80145F09, byte +0x79) holds
 *           6, it runs the shared front-end startup helper 0x8015C088 after the
 *           masked read and then requests primary state 4 and substate 5 through
 *           the shared bytes 0x80146874/0x80146875; otherwise it only performs
 *           the read-modify-write. The pointer cell 0x80146884 is re-read in
 *           both arms, so one load pair per path. Takes no arguments and returns
 *           nothing. The 0x18-byte frame keeps only $ra, whose spill fills the
 *           delay slot of the `bne` on the party byte, and the masked byte store
 *           fills the delay slot of the startup call.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearWorkAnimBit0ThenRunStartupAndRequestPrimaryState4Substate5IfPartyByte6World01Area04914_801F3884(
    void) {
  u8* record;

  if (D_80145F09 == 6) {
    record = D_80146884;
    record[0x74] &= 0xFE;
    func_8015C088();
    D_80146874 = 4;
    D_80146875 = 5;
  } else {
    record = D_80146884;
    record[0x74] &= 0xFE;
  }
}
