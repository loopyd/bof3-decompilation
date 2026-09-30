#include "bof3/scenario/scena00_internal.h"

/* @source 0x801F73A8
 * @behavior Takes no arguments and returns nothing: reseeds the scratchpad work
 * object's sprite through func_801F7134(1), then darkens that object's three
 * color channels (bytes 0x5D/0x5E/0x5F, the r0/g0/b0 the seeder copies into
 * the SPRT) by 2 each; once the channel at 0x5D reads exactly zero it clears
 * the object's byte at 0x09 and resets the shared work flags through
 * func_80196070.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801F73A8(void) {
  func_801F7134(1);
  D_1F800044[0x5D] -= 2;
  D_1F800044[0x5E] -= 2;
  D_1F800044[0x5F] -= 2;

  if (D_1F800044[0x5D] == 0) {
    D_1F800044[9] = 0;
    func_80196070();
  }
}
