#include "bof3/bof3.h"

extern u8 D_801F294D; /* @source 0x801F294D @kind data */
extern void (*D_801D4BE4[])(void); /* @source 0x801D4BE4 @kind data */

/* @source 0x801D0E80
 * @behavior Calls the handler in the table at 0x801D4BE4 indexed by the
 * unsigned byte at 0x801F294D, without passing arguments or using a result.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchByte1f294dTable4be4Commu0107_801D0E80(void) {
    D_801D4BE4[D_801F294D]();
}
