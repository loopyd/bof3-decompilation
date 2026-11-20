#include "bof3/bof3.h"

extern u8* scratchWorkRecordPointer_1F800044; /* @source 0x1F800044 @kind data */
extern u8 D_8014933B; /* @source 0x8014933B @kind data */

/* @source 0x801F3230
 * @behavior Handler entry 0 of the three-entry work-byte-2 dispatch table at
 *           0x801F4314 (entries 0x801F3230, 0x801F3278 and 0x801F3304), which
 *           the dispatcher at 0x801F31EC indexes with byte +2 of the work
 *           record published through scratchpad pointer cell 0x1F800044. While
 *           the shared overlay state byte at 0x8014933B reads 1 it clears work
 *           byte 0xB, stores zero into work word 0x3C and adds one to dispatch
 *           state byte +2, so the next step runs entry 1 at 0x801F3278. The
 *           byte store ends the cached value, so the pointer cell is loaded a
 *           second time for the remaining two accesses. Byte 0xB and word 0x3C
 *           have no established role in this overlay. Takes no arguments and
 *           returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearWorkByteBAndWord3CThenAdvanceStateByte2WhenOverlayStateIs1World03Area14313_801F3230(void) {
  u8* work;
  u8 state;

  if (D_8014933B == 1) {
    scratchWorkRecordPointer_1F800044[0xB] = 0;
    work = scratchWorkRecordPointer_1F800044;
    state = work[2];
    *(u32*)(work + 0x3C) = 0;
    work[2] = state + 1;
  }
}
