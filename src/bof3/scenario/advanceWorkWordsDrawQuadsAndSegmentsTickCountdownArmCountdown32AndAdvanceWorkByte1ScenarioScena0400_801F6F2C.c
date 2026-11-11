#include "bof3/bof3.h"

void func_801F7084(u8* record);
void func_801F75BC(u8 fraction, u8 intensity);

/* @source 0x1F800044 @kind unknown */
extern u8* D_1F800044;

/* @source 0x801F6F2C
 * @behavior Entry 3 of the nine-word work-byte-1 handler run at 0x801FA180
 *           ({0x801F6D44, 0x801F6DD4, 0x801F6EA4, 0x801F6F2C, 0x801F6FF8,
 *           0x801F7064, 0x801F77EC, 0x801F78F4, 0x801F7A64}; the word after
 *           the run at 0x801FA1A4 is 0x00010000), so the dispatcher
 *           dispatchWorkByte1HandlerScenarioScena0400_801F6D00 reaches it only
 *           through its indirect call, and entry 2 at 0x801F6EA4 arms work byte
 *           9 with 4 before it hands over. It loads the work-object pointer from
 *           the scratchpad cell 0x1F800044 and advances the two words entry 0
 *           stamped: +0x1C by 0x4000 and +0x34 by 0x10000. It then passes the
 *           record at the work object +0x0C to the quad-primitive builder at
 *           0x801F7084, re-reads the cell to take the countdown byte 9, and
 *           passes it to the five-segment drawer at 0x801F75BC as an intensity
 *           of 0xFF when byte 9 is 8 and otherwise byte 9 scaled by 32 and
 *           masked to a byte, with a constant 0x10 fraction. After that call it
 *           re-reads the cell and decrements work byte 9 in a byte-width
 *           read-modify-write (lbu / addiu -1 / sb) that it tests against zero
 *           through andi 0xFF; only when the countdown reaches zero does it
 *           re-read the cell to arm byte 9 with 0x20 and re-read it once more to
 *           increment work byte 1, so the next frame dispatches entry 4 at
 *           0x801F6FF8. The leading accesses go through the one loaded pointer
 *           while the tail re-reads the cell per access, because the stores
 *           through that pointer may touch the cell. It takes no arguments,
 *           consumes no result, and its 0x18-byte frame only keeps $ra across
 *           the two calls.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceWorkWordsDrawQuadsAndSegmentsTickCountdownArmCountdown32AndAdvanceWorkByte1ScenarioScena0400_801F6F2C(
    void) {
  u8* work;
  u8* next;
  u8 countdown;
  s32 intensity;

  work = D_1F800044;
  *(u32*)(work + 0x1C) += 0x4000;
  *(u32*)(work + 0x34) += 0x10000;
  func_801F7084(work + 0xC);
  countdown = D_1F800044[9];
  if (countdown != 8) {
    intensity = (u8)(countdown << 5);
  } else {
    intensity = 0xFF;
  }
  func_801F75BC(0x10, intensity);
  if (--D_1F800044[9] == 0) {
    next = D_1F800044;
    next[9] = 0x20;
    next = D_1F800044;
    next[1]++;
  }
}
