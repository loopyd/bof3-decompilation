#include "bof3/bof3.h"

void stateWeightedOthersDistance(
    void);

/* @source 0x801F362C
 * @behavior Thin overlay wrapper calling the helper at 0x801F3504.
 * @status exact
 * @match 100.00
 * @residual none
 */
void invokeHelper(void) {
  stateWeightedOthersDistance();
}
