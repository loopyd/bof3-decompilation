#include "bof3/world/area03004_internal.h"

/* @source 0x801DC74C
 * @behavior AREA030 work-record dispatch step, handler 0 of the local handler table at
 * 0x801E22C8 selected by byte 3 of the scratch work record published at
 * 0x1F800044: while the shared gate halfword D_80143C40 is clear it arms the
 * callback scheduler through yieldCallbackSlotScheduler(1), commits the shared fixed-point
 * channel pair D_80149308 = 0xF0000/D_8014930C = 0x3C0000, resets the overlay
 * through func_80153DE8, clears the shared front-end entry table through
 * stateHeadFieldsTwo and requests front-end
 * mode 3 through func_8014ECAC; it then requests resource 0xA with the second
 * argument 2 (func_8014D6DC) while byte 0x79 of the record at D_80146250 is
 * clear and selects active-work sequence 0xB through selectActiveWorkSequence
 * otherwise, and finally stores 1 in that dispatch byte, the step the sibling
 * handler initializeBytesGateClear continues
 * from.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateChannelPairDispatch(void) {
  u8* work;

  if (D_80143C40 == 0) {
    yieldCallbackSlotScheduler(1);
    D_80149308 = 0xF0000;
    D_8014930C = 0x3C0000;
    func_80153DE8();
    stateHeadFieldsTwo();
    func_8014ECAC(3);
    if (D_80146250[0x79] == 0) {
      func_8014D6DC(0xA, 2);
    } else {
      selectActiveWorkSequence(0xB);
    }
    work = WORLD00_AREA030_SCRATCH_PTR;
    work[3] = 1;
  }
}
