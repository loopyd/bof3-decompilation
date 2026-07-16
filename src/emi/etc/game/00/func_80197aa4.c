#include "internal.h"

/* @behavior waits for the selected frontend route, configures its scenario
 * coordinates and stream mode, then advances the New Game/load preparation
 * chain.
 * @source 0x80197AA4
 */
void func_80197AA4(void) {
  s32 local_ready;

  local_ready = func_801BEE5C();
  func_801A06D8();
  func_801992B8();
  if (local_ready) {
    func_80161808(1);
    if ((D_80146325 & 1) == 0) {
      D_8014930A += D_801C7B74[D_801462EC * 2];
      D_8014932E = 64;
      D_80149330 = (s32)(D_801CD954 - D_80149318) >> 4;
      D_8014930E += D_801C7B74[D_801462EC * 2 + 1];
    }
    if ((D_80146325 & 0x10) == 0) {
      if ((D_801462EC & 2) != 0) {
        func_8016728C(D_80145024 & 0x7f, 2);
      } else {
        func_8016728C(D_80145024 & 0x7f, 1);
      }
    }
    D_8014933E = 4;
    D_80149333 = 2;
    D_8014933F = D_801462EC & 1;
    D_80143B92++;
  }
}
