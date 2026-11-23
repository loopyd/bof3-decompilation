#include "bof3/bof3.h"

extern u16 D_80145AA4;
extern u8 D_801462E4;

/* @source 0x801EEEE0
 * @behavior Battle substate advance on input: when the shared pad state
 *   halfword at 0x80145AA4 reads nonzero, advances the battle substate index
 *   byte at 0x801462E4 by one through a pointer to that byte and leaves the
 *   battle state byte at 0x801462E5 untouched; takes no arguments, returns
 *   nothing and touches no stack.
 * @status exact
 * @match 100.00
 * @residual none
 * 0x801EEEE0 is the word at 0x801F0CAC, entry 2 of the handler run based at
 * 0x801F0CA4 (entry 5 of the fourteen-pointer run beginning at 0x801F0C98), so
 * dispatchSubstateTable0ca4BatlEnd00_801EEC94 reaches it with the substate index
 * it then advances. The no-argument, no-frame shape and the pointer-held store
 * are this span's own; the guard and statement order follow the exact sibling
 * advanceSubstateWhenPadActiveBatlEnd00_801EF77C in this target, which adds the
 * state-byte clear at 0x801462E5 this body does not have.
 */
void stateSubstatePadActive(void) {
  u8 *substate;

  if (D_80145AA4 != 0) {
    substate = &D_801462E4;
    *substate += 1;
  }
}
