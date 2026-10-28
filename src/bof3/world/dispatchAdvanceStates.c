#include "bof3/world/area01613_internal.h"

/*
 * @behavior dispatch both advance states in order.
 * @source 0x801F34A0
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchAdvanceStates(void) {
    dispatchState02();
    dispatchState03();
}
