#include "bof3/battle/battle15_internal.h"

/* @source 0x8009F83C
 * @behavior Forwards request kind 2 to the battle selection query
 * func_800A4294 and stores the signed halfword result in the active selection
 * record.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_8009F83C(void) {
  D_801463A0[2] = func_800A4294(2);
}
