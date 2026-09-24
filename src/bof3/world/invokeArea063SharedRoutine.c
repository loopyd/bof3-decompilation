#include "bof3/bof3.h"

void func_801F2C04(void);

/* @source 0x801F2D2C
 * @behavior Calls the sibling overlay routine at 0x801F2C04 with this entry's own
 * incoming argument registers left untouched and returns that routine's result
 * unchanged; no other work is performed.
 * @status exact
 * @match 100.00
 * @residual none
 */
void invokeArea063SharedRoutine(void) {
  func_801F2C04();
}
