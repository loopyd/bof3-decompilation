#include "bof3/bof3.h"

extern u8 *workRecord801F24E0; /* @source 0x801F24E0 @kind data */
extern void (*dispatchHandlerTable801F1C30[])(void); /* @source 0x801F1C30 @kind data */

/* @source 0x801F10F4
 * @behavior Reads the record pointer at 0x801F24E0, uses its unsigned byte at
 * offset 1 to index the handler table at 0x801F1C30, and invokes that handler
 * without arguments. The dispatcher itself does not store to the record.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchRecordByte1HandlerBmagicMagic13103_801F10F4(void) {
  dispatchHandlerTable801F1C30[workRecord801F24E0[1]]();
}
