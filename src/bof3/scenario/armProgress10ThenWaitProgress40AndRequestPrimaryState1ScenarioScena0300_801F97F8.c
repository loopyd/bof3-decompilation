#include "bof3/bof3.h"

extern u8 D_80146874;
extern u8 D_80146875;
extern u8 g_ScenarioProgress;
void func_801BE1B0(u32 arg0);

/* @source 0x801F97F8
 * @behavior Slot 2 of the overlay's nine-entry primary-state handler table
 * D_801FCFE0, i.e. word 25 (0x801FCFE8) of the 32-word code-pointer run at
 * 0x801FCF84 that dispatchPrimaryStateHandlerScenarioScena0300_801F92F4
 * indexes with the signed shared primary state byte D_80146874; the overlay
 * reaches it only through that indirect call, so no in-image jal names it.
 * It switches on the shared sub-state byte D_80146875, loaded once into a
 * register before the frame is created: sub-state 0 runs the main-exe helper
 * func_801BE1B0(1), stores 10 in the shared scenario progress byte
 * g_ScenarioProgress and advances the sub-state to 1; sub-state 1 waits until
 * that progress byte reads 0x28 (40) and then requests primary state 1 by
 * storing 1 in D_80146874, holding the sub-state at 1; any other sub-state
 * value returns without a store. Both storing arms share the single
 * sub-state store, which is why the byte is written once at the merged tail
 * and the non-storing arms jump past it. It reads no other state, takes no
 * arguments and returns nothing of its own (the caller's table is a
 * void(void) pointer run); the 0x18-byte frame only keeps $ra and the one
 * saved sub-state base register. The constant 1 left in $v0 on every path is
 * the byte-store operand, not a returned value.
 * @status exact
 * @match 100.00
 * @residual none
 */
void armProgress10ThenWaitProgress40AndRequestPrimaryState1ScenarioScena0300_801F97F8(
    void) {
  u8 *subState;
  u8 next;

  subState = &D_80146875;
  switch (*subState) {
  case 0:
    func_801BE1B0(1);
    g_ScenarioProgress = 10;
    next = 1;
    break;

  case 1:
    if (g_ScenarioProgress != 0x28) {
      return;
    }
    next = 1;
    D_80146874 = next;
    break;

  default:
    return;
  }
  *subState = next;
}
