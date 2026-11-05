#include "bof3/bof3.h"

extern u8 D_801D4E08; /* @source 0x801D4E08 @kind data */
extern void (*D_801D4BD8[])(void); /* @source 0x801D4BD8 @kind data */

/* @source 0x801D0E44
 * @behavior Calls the handler in the table at 0x801D4BD8 indexed by the
 * unsigned byte at 0x801D4E08, without passing arguments or using a result.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchByte4e08Table4bd8Commu0107_801D0E44(void) {
    D_801D4BD8[D_801D4E08]();
}
