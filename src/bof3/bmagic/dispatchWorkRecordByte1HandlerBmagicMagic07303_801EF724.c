#include "bof3/bof3.h"

extern u8 *D_801F21F0; /* @source 0x801F21F0 @kind data */
extern void (*D_801F066C[])(void); /* @source 0x801F066C @kind data */

/* @source 0x801EF724
 * @behavior Dispatches the current bmagic work record through the local handler
 * table at 0x801F066C. The caller publishes the record pointer at 0x801F21F0;
 * its unsigned byte at offset 1 selects the handler, which is called without
 * arguments. The dispatcher performs no stores and ignores the handler result.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkRecordByte1HandlerBmagicMagic07303_801EF724(void) {
    D_801F066C[D_801F21F0[1]]();
}
