#include "bof3/bof3.h"

/* @source 0x80145F09 @kind unknown - party byte (+0x79) of the 0x140-byte
 * stride local work records based at 0x80145E90, so this reads record 0. */
extern u8 D_80145F09;
extern void* D_80146884; /* @source 0x80146884 @kind data */
/* @source 0x80146874 @kind unknown */
extern s8 D_80146874;
/* @source 0x80146875 @kind unknown */
extern u8 D_80146875;
/* @source 0x80146864 @kind data */
extern u8 g_ScenarioProgress;
/* @source 0x80143FC8 */
extern u8 D_80143FC8[];
s32 func_8019601C(void);

void func_8015DF18(u32 cue);
void func_801A4BC0(s16 x, s16 y, u32 size);

/* @source 0x801F360C
 * @behavior Clears bit 0 of the byte at offset 0x74 of the work record
 *           published at 0x80146884. When the party byte of local work record 0
 *           (0x80145F09, byte +0x79) holds 5, it first claims a front-end record
 *           slot with func_8019601C, masks the returned index to a byte and,
 *           unless that index is 0xFF, marks the selected record of the
 *           0x74-byte stride front-end table at 0x80143FC8 with 1 at +0 and kind
 *           0x6C at +5, then queues front-end cue 0x209 through func_8015DF18,
 *           publishes the two shared UI elements (0x0B, 0x55) and (0x0C, 0x55)
 *           with size 0xC0 through func_801A4BC0 and requests primary state 4 and
 *           substate 0x1E through 0x80146874/0x80146875 while clearing
 *           g_ScenarioProgress; otherwise it only performs the masked
 *           read-modify-write. The pointer cell 0x80146884 is re-read in both
 *           arms, so one load pair per path, and the seeded front-end record
 *           pointer is a separate pseudo, which is what keeps the sum in $v0 and
 *           the stored constants in $v1. Takes no arguments and returns nothing.
 *           The 0x18-byte frame keeps only $ra, whose spill fills the delay slot
 *           of the `bne` on the party byte, and the masked byte store fills the
 *           delay slot of the first UI call.
 * @status exact
 * @match 100.00
 * @residual none
 */
void seedFrontEndRecordKind6CQueueCue209AndClearWorkAnimBit0ThenRequestPrimaryState4Substate30IfPartyByte5World01Area04914_801F360C(
    void) {
  u8* record;
  u8* frontEndRecord;
  s32 index;

  if (D_80145F09 == 5) {
    index = func_8019601C() & 0xFF;
    if (index != 0xFF) {
      frontEndRecord = D_80143FC8 + index * 0x74;
      frontEndRecord[0] = 1;
      frontEndRecord[5] = 0x6C;
    }
    func_8015DF18(0x209);
    record = D_80146884;
    record[0x74] &= 0xFE;
    func_801A4BC0(0x0B, 0x55, 0xC0);
    func_801A4BC0(0x0C, 0x55, 0xC0);
    D_80146874 = 4;
    g_ScenarioProgress = 0;
    D_80146875 = 0x1E;
  } else {
    record = D_80146884;
    record[0x74] &= 0xFE;
  }
}
