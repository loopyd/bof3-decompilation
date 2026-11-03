#include "bof3/bof3.h"

/*
 * Code-pointer table at 0x801EFE58, the table this dispatcher indexes:
 * entry 0 = 0x801EF910, entry 1 = 0x801EFAF4, entry 2 = 0x801EFBA8,
 * entry 3 = 0x801EFBEC, entry 4 = 0x801EFD84.
 */
extern void (*func_801EFE58[])(void); /* @source 0x801EFE58 @kind data */

/* @source 0x801EF8CC
 * @behavior Dispatches one frame of the overlay work handler from the
 * scratchpad work object: the work-object pointer cell at 0x1F800044 is read,
 * its state byte at offset 1 is scaled by four and selects an entry of the
 * five-entry overlay-local code-pointer table at 0x801EFE58 (0x801EF910,
 * 0x801EFAF4, 0x801EFBA8, 0x801EFBEC, 0x801EFD84), and the selected entry is
 * invoked with no arguments. It reads no other state and writes none; the
 * 0x18-byte frame only keeps $ra across the indirect call.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte1HandlerBmagicMagic00403_801EF8CC(void) {
  func_801EFE58[SPAD_PTR_SLOT(u8, 0x44u)[1]]();
}
