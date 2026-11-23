#include "bof3/world/area03004_internal.h"

/* @source 0x801DB554
 * @behavior When the shared count is 4, compare the indexed byte with the selected
 * record's unsigned halfword at 0x90. If the byte is smaller, copy the
 * halfword's low byte and set D_801E3214. Request cue 0x102, seed countdown 8
 * and advance a fresh work byte 3. Always step the pair sequence.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateIndexedCountFour(void) {
  u8* dispatchWork;
  u8 valueIndex;
  u32 recordValue;

  if (D_80144199_BYTE == 4) {
    valueIndex = D_80146888[D_801E3208].unk_02[4];
    recordValue = *(u16*)&D_80146888[D_801E3208].unk_90[0];
    if (D_80144FE4[valueIndex] < recordValue) {
      D_80144FE4[valueIndex] = recordValue;
      D_801E3214 = 1;
    }
    func_8015DF18(0x102);
    dispatchWork = D_1F800044;
    dispatchWork[9] = 8;
    dispatchWork = D_1F800044;
    dispatchWork[3]++;
  }
  func_8014D978();
}
