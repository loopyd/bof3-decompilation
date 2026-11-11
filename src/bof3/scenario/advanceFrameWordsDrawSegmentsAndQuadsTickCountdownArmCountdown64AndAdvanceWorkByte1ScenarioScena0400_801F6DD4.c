#include "bof3/bof3.h"

void func_801F75BC(u8 fraction, u8 intensity);
void func_801F7084(u8* record);

/* @source 0x1F800044 @kind unknown */
extern u8* D_1F800044;

/* @source 0x801F6DD4
 * @behavior Entry 1 of this overlay's nine-word work-byte-1 handler run at
 *           0x801FA180 ({0x801F6D44, 0x801F6DD4, 0x801F6EA4, 0x801F6F2C,
 *           0x801F6FF8, 0x801F7064, 0x801F77EC, 0x801F78F4, 0x801F7A64}), so
 *           the dispatcher dispatchWorkByte1HandlerScenarioScena0400_801F6D00
 *           reaches it only through its indirect call; entry 0
 *           (seedWorkFieldsAndSegmentKeyTableSetByte9AndAdvanceWorkByte1ScenarioScena0400_801F6D44)
 *           leaves work byte 1 at 1 with work byte 9 armed at 8, so every frame
 *           that holds byte 1 at 1 runs one step of that eight-frame countdown.
 *           It loads the work-object pointer from the scratchpad cell
 *           0x1F800044 once and keeps it in a saved register across both calls.
 *           From the countdown byte it builds a 0..16 interpolation fraction,
 *           0x10 - 2 * byte 9 masked to even, and passes it with intensity 0xFF
 *           to the five-segment drawer at 0x801F75BC, which walks the key table
 *           at 0x801FA2C0 one record per segment. Before that call it advances
 *           three of the words entry 0 stamped into the work object: +0x24 by
 *           0x10000, +0x34 by 0x2000 and +0x38 by 0x220000; those three updates
 *           are independent, so the original emits the +0x34 store first even
 *           though this source order is what schedules into it. It then passes
 *           the record at the work object's +0x0C to the quad builder at
 *           0x801F7084; that record pointer is taken before the first call, so
 *           the advance sits in that call's delay slot on the saved register.
 *           After both calls it re-reads the cell and decrements work byte 9 in
 *           a byte-width read-modify-write (lbu / addiu -1 / sb) that it tests
 *           against zero through andi 0xFF. Only when the countdown reaches
 *           zero does it re-read the cell and arm byte 9 with 0x40, then
 *           re-read it once more to increment work byte 1, so the next frame
 *           dispatches entry 2 at 0x801F6EA4. The leading accesses go through
 *           the saved pointer while the tail re-reads the cell per access,
 *           because the stores through that pointer may touch the cell. It
 *           takes no arguments, consumes no result, and its 0x18-byte frame
 *           keeps $s0 and $ra across the two calls.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceFrameWordsDrawSegmentsAndQuadsTickCountdownArmCountdown64AndAdvanceWorkByte1ScenarioScena0400_801F6DD4(
    void) {
  u8* work;
  u8* record;
  u8* next;
  s32 fraction;

  work = D_1F800044;
  fraction = (0x10 - work[9] * 2) & 0xFE;
  *(u32*)(work + 0x24) += 0x10000;
  *(u32*)(work + 0x34) += 0x2000;
  *(u32*)(work + 0x38) += 0x220000;

  record = work + 0x0C;
  func_801F75BC(fraction, 0xFF);
  func_801F7084(record);
  if (--D_1F800044[9] == 0) {
    next = D_1F800044;
    next[9] = 0x40;
    next = D_1F800044;
    next[1]++;
  }
}
