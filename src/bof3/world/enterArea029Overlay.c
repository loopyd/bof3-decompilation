#include "bof3/bof3.h"

extern void func_801F2C04();

/* @source 0x801F2D2C
 * @behavior Overlay entry thunk referenced by this payload's trailing pointer word at
 * 0x801F2E14: it calls the overlay body func_801F2C04 with the incoming argument
 * registers left untouched and returns to its caller with that call's result.
 * @status exact
 * @match 100.00
 * @residual none
 */
void enterArea029Overlay(void) {
  func_801F2C04();
}
