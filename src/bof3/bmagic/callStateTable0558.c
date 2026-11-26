#include "bof3/bof3.h"

extern u8 *stateWorkRecordPointer; /* @source 0x801F20DC @kind data */
extern void (*tableStageHandlers[])(void); /* @source 0x801F0558 @kind data */

/* @source 0x801EF710
 * @behavior Dispatches the current work record through the local handler table
 * at 0x801F0558. The caller publishes the record pointer at 0x801F20DC;
 * its unsigned byte at offset 1 selects the handler, called without arguments.
 * Performs no stores and ignores the handler result.
 * @status exact
 * @match 100.00
 * @residual none
 */
void callStateTable0558(void) {
    tableStageHandlers[stateWorkRecordPointer[1]]();
}
