#include "bof3/world/area03004_internal.h"

/* @behavior AREA030 panel state handler selected by the overlay's handler
 * pointer word at 0x801E1FE0: calls the shared helper func_8014D978, then
 * func_8014D4E0, and returns. Neither call is given an argument from this
 * site and neither result is used here.
 * @source 0x801D5B48
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801D5B48(void) {
  func_8014D978();
  func_8014D4E0();
}
