#include "bof3/bof3.h"

/* @source 0x801FB390
 * @behavior Increments the unsigned 16-bit work-object field at offset 0x7E
 * and stores it back through the same pointer; takes the work object in $a0
 * and returns nothing. The overlay's handler table at 0x801FBFB8 holds it as
 * entry 0.
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementWorkHalfword7EScenarioScena1300_801FB390(u8 *work) {
  *(u16 *)(work + 0x7E) += 1;
}
