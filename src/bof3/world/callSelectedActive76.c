#include "bof3/bof3.h"

void markSelectedActiveValue(s32 arg0);

/* @source 0x801F60A4
 * @behavior Thin overlay wrapper: calls the helper at 0x801F61F0 and returns.
 * @status exact
 * @match 100.00
 * @residual none
 */
void callSelectedActive76(void) {
  markSelectedActiveValue(76);
}
