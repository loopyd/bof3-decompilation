#include "bof3/bof3.h"

void activateSixCodeBank(
    void);

/* @source 0x801F3424
 * @behavior Thin overlay wrapper: calls the helper at 0x801F3444 and returns.
 * @status exact
 * @match 100.00
 * @residual none
 */
void invokeHelper(void) {
  activateSixCodeBank();
}
