#include "bof3/ui/game00_internal.h"

/**
 * @source 0x801992B8
 * @behavior Runs the ordered frame-finalization service sequence.
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void func_801992B8(void) {
  func_801527E4();
  func_8015A758();
  func_801BDAB8();
  func_801A0514();
  func_8019A0E4();
  func_8019625C();
  func_8014BA54();
}
