#include "bof3/world/area03004_internal.h"

/* @source 0x801DC74C
 * @behavior AREA030 work-record dispatch step, handler 0 of the local handler
 * table at 0x801E22C8 selected by byte 3 of the scratch work record published
 * at 0x1F800044: while the shared gate halfword D_80143C40 is clear it arms the
 * callback scheduler through func_8014B87C(1), commits the shared fixed-point
 * channel pair D_80149308 = 0xF0000/D_8014930C = 0x3C0000, resets the overlay
 * through func_80153DE8, clears the shared front-end entry table through
 * func_801E0A30 and requests front-end mode 3 through func_8014ECAC; it then
 * requests resource 0xA with the second argument 2 (func_8014D6DC) while byte
 * 0x79 of the record at D_80146250 is clear and resource 0xB (func_8014D6B8)
 * otherwise, and finally stores 1 in that dispatch byte, the step the sibling
 * handler func_801DC7EC continues from.
 * @status exact
 * @match 100.00
 * @residual none
 */
void seedChannelPairAndAdvanceWorkDispatchWorld00Area03004_801DC74C(void) {
  u8* work;

  if (D_80143C40 == 0) {
    func_8014B87C(1);
    D_80149308 = 0xF0000;
    D_8014930C = 0x3C0000;
    func_80153DE8();
    func_801E0A30();
    func_8014ECAC(3);
    if (D_80146250[0x79] == 0) {
      func_8014D6DC(0xA, 2);
    } else {
      func_8014D6B8(0xB);
    }
    work = WORLD00_AREA030_SCRATCH_PTR;
    work[3] = 1;
  }
}
