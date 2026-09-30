#include "bof3/world/area03005_internal.h"

/* @behavior dispatches the phase byte `handlerPhase` through the AREA030/05
 * handler table at the measured entry-3 base (handlerTable[3 + phase]), relays
 * the fixed panel arguments (0x14, 0x12, 0x118, 0x13, 0) to the companion
 * overlay helper at 0x801D9534, then runs the three local setup phases with a
 * zero argument.
 * @source 0x800F5048
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_800F5048(void) {
  handlerTable[3 + handlerPhase]();
  func_801D9534(0x14, 0x12, 0x118, 0x13, 0);
  func_800F606C(0);
  func_800F6730();
  func_800F5E44();
}
