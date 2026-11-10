#include "bof3/bof3.h"

extern u8 func_8014DAEC(void); /* @source 0x8014DAEC @kind function */
extern void func_801E5988(void); /* @source 0x801E5988 @kind function */
extern void func_8014D290(void); /* @source 0x8014D290 @kind function */

/* @source 0x801F0E4C
 * @behavior Calls the shared readiness helper at 0x8014DAEC and tests its
 * unsigned byte result. Calls helper 0x801E5988 when nonzero, otherwise calls
 * resident helper 0x8014D290. Takes no arguments and returns nothing; entry 1
 * of the handler table at 0x801F123C invokes it through work byte 2.
 * The helper at 0x801E5988 keeps its address-based name because this payload
 * does not establish its ownership or role.
 * @status exact
 * @match 100.00
 * @residual none
 */
void call801E5988WhenReadyOtherwiseCall8014D290BmagicMagic11403_801F0E4C(void) {
  if (func_8014DAEC() != 0u) {
    func_801E5988();
  } else {
    func_8014D290();
  }
}
