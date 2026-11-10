#include "bof3/bof3.h"

extern u8* scratchWorkRecordPointer_1F800044; /* @source 0x1F800044 @kind data */
extern void func_80196718(void* record); /* @source 0x80196718 */

/* @source 0x801F3C40
 * @behavior Clears byte 0x48 of the work record published through scratchpad
 * pointer cell 0x1F800044, then reloads that cell and submits the record through
 * func_80196718. The action table at 0x801F52AC holds this handler's address.
 * The role of byte 0x48 is unknown. Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearWorkByte48ThenSubmitRecordWorld03Area14313_801F3C40(void) {
  scratchWorkRecordPointer_1F800044[0x48] = 0;
  func_80196718(scratchWorkRecordPointer_1F800044);
}
