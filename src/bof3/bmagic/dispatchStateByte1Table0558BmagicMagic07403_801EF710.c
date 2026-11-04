#include "bof3/bof3.h"

extern u8 *D_801F20DC; /* @source 0x801F20DC @kind data */
extern void (*D_801F0558[])(void); /* @source 0x801F0558 @kind data */

/* @source 0x801EF710
 * @behavior Dispatches the current work record through the local handler table
 * at 0x801F0558. The caller publishes the record pointer at 0x801F20DC;
 * its unsigned byte at offset 1 selects the handler, called without arguments.
 * Performs no stores and ignores the handler result.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchStateByte1Table0558BmagicMagic07403_801EF710(void) {
    D_801F0558[D_801F20DC[1]]();
}
