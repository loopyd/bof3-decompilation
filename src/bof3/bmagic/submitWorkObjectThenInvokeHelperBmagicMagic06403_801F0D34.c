#include "bof3/bof3.h"

/* Shared function at 0x80196718: the exact sibling lift
 * func_801F35B4_area03213.c (emi/world00/area032/13@0x801F35B4) describes the
 * same address as the call that submits a work record, and
 * include/bof3/battle/battle03_internal.h declares it `void func_80196718(void*)`. */
void func_80196718(void* arg0);

/* Shared overlay helper at 0x801E5988, lifted by invokeBmagicHelperMagic004.c
 * (emi/bmagic/magic004/03@0x801EFAD4) and invoked by several exact bmagic
 * handlers of this module. */
void func_801E5988(void);

/* @source 0x801F0D34
 * @behavior Reads the scratchpad work-object pointer cell at 0x1F800044 once
 * and passes that work object to the shared function at 0x80196718, then
 * invokes the shared helper at 0x801E5988 with no arguments. Nothing is
 * returned and no other state is read or written; the work-object load is
 * issued before the 0x18-byte frame is created and that frame only keeps $ra
 * across the two calls, each of which has a nop delay slot.
 * @status exact
 * @match 100.00
 * @residual none
 */
void submitWorkObjectThenInvokeHelperBmagicMagic06403_801F0D34(void) {
  func_80196718(SPAD_PTR_SLOT(u8, 0x44u));
  func_801E5988();
}
