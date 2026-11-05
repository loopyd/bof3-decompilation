#include "bof3/bof3.h"

extern u8 D_80146875;
extern u8 D_80143BB0;
extern s8 D_80146874;

/* GAME.EMI entry 0 helper at 0x801BE1B0. */
void func_801BE1B0(u32 arg0);

/* @source 0x801FBD0C
 * @behavior When the shared substate byte D_80146875 is zero and the
 * unsigned mode byte D_80143BB0 is not 2, invokes func_801BE1B0(1), then
 * clears the primary state byte D_80146874 and the substate byte, in that
 * order. Otherwise does nothing. Takes no arguments and returns nothing;
 * the substate address survives the call for the final byte store.
 * @status exact
 * @match 100.00
 * @residual none
 */
void invokeHelper1AndClearStateUnlessMode2ScenarioScena0900_801FBD0C(void) {
  u8 *state = &D_80146875;

  if (*state == 0 && D_80143BB0 != 2) {
    func_801BE1B0(1u);
    D_80146874 = 0;
    *state = 0;
  }
}
