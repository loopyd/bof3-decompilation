#include "bof3/bof3.h"

/* Work-object pointer cell in scratchpad. The original loads the cell once and
 * stores the byte through that one pointer, so the source reads the cell once. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind data */

/* Shared main-program readiness helper, outside this payload: it answers an
 * unsigned byte and this body tests only that byte for zero. */
extern u8 func_8014DAEC(void); /* @source 0x8014DAEC */

/* @source 0x800C18B8
 * @behavior Step handler of this overlay: it calls the shared readiness helper
 * at 0x8014DAEC and, when the unsigned byte that helper answers is nonzero,
 * stores 2 into work-record byte 1 of the work object held by the scratchpad
 * pointer cell 0x1F800044. Byte 1 is the workspace-state byte the dispatcher at
 * 0x800C1804 indexes the handler run at 0x800C1B2C with, so this body re-arms
 * that dispatch state to slot 2 after the helper reports ready. The original
 * keeps the ready flag in $v0, materializes the constant 2 in the branch delay
 * slot, loads the pointer cell once into $v1, leaves the load delay slot after
 * that load and the return delay slot as nops, writes no other state and leaves
 * 2 in $v0. Its 0x3C-byte span carries no target-local immediate: a payload cmp
 * showed the bytes of this span identical to the unlifted spans at 0x800C1940
 * and 0x800C1ADC of emi/boss/boss018/16, which hold the same helper call, test
 * and byte store. Roles of work-record byte 1 beyond this dispatch state are not
 * established here.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateHelperReadyBoss(void) {
  if (func_8014DAEC()) {
    D_1F800044[1] = 2u;
  }
}
