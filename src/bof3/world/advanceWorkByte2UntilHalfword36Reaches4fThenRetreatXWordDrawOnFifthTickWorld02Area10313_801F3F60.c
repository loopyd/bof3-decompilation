#include "bof3/bof3.h"

/* Scratchpad work-record pointer cell. Naming the cell keeps the original's
 * per-access re-materialization (lui $vN,0x1F80 / lw $vN,68($vN)); the bare
 * constant address makes the compiler CSE 0x1F800044 into one saved register
 * and load through offset zero instead.
 * @source 0x1F800044 @kind data
 */
extern u8* D_1F800044;

/* Shared per-frame counter, incremented once for each displayed frame. */
extern u32 D_80143E6C;

void func_801F4E70(u32 variant);
void func_801F49A0(u32 pass);
void func_801F482C(void);

/* @source 0x801F3F60
 * @behavior Overlay step handler, entry 5 of the work-byte-2 handler table at
 *           0x801F5954 and entry 8 of the byte-1 table at 0x801F5948. It reads
 *           the work record published at the scratchpad pointer cell
 *           0x1F800044 and watches the signed halfword at 0x36, the upper half
 *           of the fixed-point x coordinate word at 0x34: while that halfword
 *           stays below 0x4F the handler advances work byte 0x02 by one on
 *           every frame whose shared counter at 0x80143E6C satisfies
 *           ((counter / 5) & 3) == 3, so the next frame dispatches entry 6;
 *           once the halfword has reached 0x4F it retreats the fixed-point x
 *           word at 0x34 by 0x1000 per frame instead. Every frame then
 *           re-reads the shared counter and, when it is a multiple of five,
 *           drives the block drawer func_801F4E70 with the four-way variant
 *           (counter / 5) & 3, and ends with func_801F49A0(0),
 *           func_801F49A0(1) and the four-block drawer func_801F482C, which
 *           emits four SetDrawMove blocks anchored on the record's fixed-point
 *           x/y pair at 0x34/0x38. Takes no arguments, returns nothing and uses
 *           a 0x18-byte frame that holds only the saved return address at 0x10.
 *           The roles of the halfword at 0x36 and of the three callees beyond
 *           their drawing effects stay described by offset and address.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceWorkByte2UntilHalfword36Reaches4fThenRetreatXWordDrawOnFifthTickWorld02Area10313_801F3F60(
    void) {
  u8* work;
  u32 frame;

  work = D_1F800044;
  if (*(s16*)(work + 0x36) >= 0x4F) {
    *(s32*)(work + 0x34) -= 0x1000;
  } else {
    frame = D_80143E6C;
    if (((frame / 5) & 3) == 3) {
      work[2]++;
    }
  }
  frame = D_80143E6C;
  if (frame % 5 == 0) {
    func_801F4E70((frame / 5) & 3);
  }
  func_801F49A0(0);
  func_801F49A0(1);
  func_801F482C();
}
