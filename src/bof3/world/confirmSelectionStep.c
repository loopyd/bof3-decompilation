#include "bof3/world/area03004_internal.h"

/* @behavior AREA030 selection-confirm step: requests the shared selection
 * effect 9 through func_8014D8D4 and cue 0x204 through func_8015DF18, stores
 * the AREA030 mode 7 in the shared mode byte and 3 in its companion byte
 * 0x8014419E, advances the shared selection counter byte 0x80144199 and
 * dispatch byte 3 of the scratch work record published at the scratchpad
 * cursor 0x1F800044, then releases the shared hold through func_8014D978.
 * @source 0x801DDDBC
 * @status exact
 * @match 100.00
 * @residual none
 */
void confirmSelectionStep(void) {
  u8* work;

  func_8014D8D4(9u);
  func_8015DF18(0x204u);
  modeByte = 7u;
  D_8014419E = 3u;
  D_80144199_BYTE = (u8)(D_80144199_BYTE + 1u);
  work = WORLD00_AREA030_SCRATCH_PTR;
  work[3] = (u8)(work[3] + 1u);
  func_8014D978();
}
