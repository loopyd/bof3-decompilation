#include "bof3/world/area03004_internal.h"

/* @source 0x801DAFBC
 * @behavior When D_80144199_BYTE is 3, call func_8015D404(0, 0) and func_80161C20(0x29,
 * 100, 8), request state 2, advance D_8014420D, clear work byte 9 and advance
 * work byte 3. Always call the pair-sequence step func_8014D978 afterwards.
 * @status exact
 * @match 100.00
 * @residual none
 */
void requestTwoThreeSequence(void) {
  u8* work;

  if (D_80144199_BYTE == 3) {
    func_8015D404(0, 0);
    func_80161C20(0x29, 100, 8);
    stateRequestedSelectedModes(2);
    D_8014420D++;
    work = D_1F800044;
    work[9] = 0;
    work = D_1F800044;
    work[3]++;
  }
  func_8014D978();
}
