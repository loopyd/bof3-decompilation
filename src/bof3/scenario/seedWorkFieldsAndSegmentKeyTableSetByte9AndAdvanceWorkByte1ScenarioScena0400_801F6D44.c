#include "bof3/bof3.h"

/* @source 0x1F800044 @kind unknown */
extern u8* D_1F800044;

void seedSegmentKeyTableAt801FA2C0ScenarioScena0400_801F747C(void);

/* @source 0x801F6D44
 * @behavior Step-0 handler of this overlay's work-byte-1 handler run at
 *           0x801FA180: word 0 of the nine entries 0x801F6D44 through
 *           0x801F7A64 that dispatchWorkByte1HandlerScenarioScena0400_801F6D00
 *           indexes with that work byte, and reached in image only through that
 *           indirect call. It loads the work-object pointer from the scratchpad
 *           cell 0x1F800044, stamps ten words into it, calls the segment-key
 *           seeder at 0x801F747C, then arms work byte 9 with 8 and advances work
 *           byte 1 modulo 256, so the next frame dispatches entry 1
 *           (0x801F6DD4). The stamped words are 0x568000 at +0x0C and +0x24,
 *           0x128000 at +0x10 and +0x28, 0x1000000 at +0x14 and +0x2C, 0 at
 *           +0x1C, 0x2200000 at +0x20 and +0x38 and 0x4000 at +0x34, while the
 *           words at +0x18 and +0x30 stay untouched; those stores are
 *           independent, so the ascending-offset source order below schedules
 *           into the original +0x14, +0x2C, +0x0C, +0x10, +0x1C, +0x20, +0x24,
 *           +0x28, +0x34, +0x38 emission, which leaves the +0x38 store in the
 *           call delay slot on the pre-call pointer. The pointer cell is read
 *           per access because each store through the loaded pointer may touch
 *           it, so the original re-materialises `lui $v1,0x1F80` /
 *           `lw $v1,0x44($v1)` three times instead of holding the cell address
 *           in a saved register. The image holds no `jal` to this address and
 *           one word equal to it, so the run word above is its only reference.
 *           It takes no arguments, consumes no result, and its 0x18-byte frame
 *           only keeps $ra across the call.
 * @status exact
 * @match 100.00
 * @residual none
 */
void seedWorkFieldsAndSegmentKeyTableSetByte9AndAdvanceWorkByte1ScenarioScena0400_801F6D44(
    void) {
  u8* work;
  u8* next;

  work = D_1F800044;
  *(u32*)(work + 0x0C) = 0x568000;
  *(u32*)(work + 0x10) = 0x128000;
  *(u32*)(work + 0x14) = 0x1000000;
  *(u32*)(work + 0x1C) = 0;
  *(u32*)(work + 0x20) = 0x2200000;
  *(u32*)(work + 0x24) = 0x568000;
  *(u32*)(work + 0x28) = 0x128000;
  *(u32*)(work + 0x2C) = 0x1000000;
  *(u32*)(work + 0x34) = 0x4000;
  *(u32*)(work + 0x38) = 0x2200000;
  seedSegmentKeyTableAt801FA2C0ScenarioScena0400_801F747C();
  next = D_1F800044;
  next[9] = 8;
  next = D_1F800044;
  next[1]++;
}
