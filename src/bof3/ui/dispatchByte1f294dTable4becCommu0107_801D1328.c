#include "bof3/bof3.h"

extern u8 D_801F294D; /* @source 0x801F294D @kind data */
extern void (*D_801D4BEC[])(void); /* @source 0x801D4BEC @kind data */

/* @source 0x801D1328
 * @behavior Calls the handler in the table at 0x801D4BEC indexed by the
 * unsigned byte at 0x801F294D, without passing arguments or using a result.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchByte1f294dTable4becCommu0107_801D1328(void) {
    D_801D4BEC[D_801F294D]();
}
