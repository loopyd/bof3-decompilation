#include "internal.h"

/* @behavior waits for the selected frontend route, configures its scenario
 * coordinates and stream mode, then advances the New Game/load preparation
 * chain.
 * @source 0x80197aa4 func_80197aa4
 */
void func_80197aa4(void) {
  s32 local_ready;

  local_ready = func_801bee5c();
  func_801a06d8();
  func_801992b8();
  if (local_ready) {
    func_80161808(1);
    if ((DAT_80146325 & 1) == 0) {
      DAT_8014930a += DAT_801c7b74[DAT_801462ec * 2];
      DAT_8014932e = 64;
      DAT_80149330 = (s32)(DAT_801cd954 - DAT_80149318) >> 4;
      DAT_8014930e += DAT_801c7b74[DAT_801462ec * 2 + 1];
    }
    if ((DAT_80146325 & 0x10) == 0) {
      if ((DAT_801462ec & 2) != 0) {
        func_8016728c(DAT_80145024 & 0x7f, 2);
      } else {
        func_8016728c(DAT_80145024 & 0x7f, 1);
      }
    }
    DAT_8014933e = 4;
    DAT_80149333 = 2;
    DAT_8014933f = DAT_801462ec & 1;
    DAT_80143b92++;
  }
}
