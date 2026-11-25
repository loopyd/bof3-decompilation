#include "bof3/bof3.h"

extern s8 D_801490C7;
extern u16 D_801490A8;
extern u8 D_801448EB;
extern u8 D_801448EC;

/* The word handler table based at 0x801F64B8: sixteen argument-free entry
 * pointers, entries 0..3 the status/mode handlers 0x801F5A68, 0x801F5A98,
 * 0x801F5AC8 and 0x801F5B0C and entry 4 this file's body, which is reached only
 * through that table. */
extern void (*statusModeHandlerTable[])(void); /* @source 0x801F64B8 @kind data */

void func_8015C088(void);

/* @source 0x801F5B50
 * @behavior Overlay handler: stores 0xFFFF into the shared status halfword at
 *           0x801490A8 on every path (`ori v0,0,0xFFFF` then `sh`), reads the
 *           shared gate byte at 0x801490C7 with a signed byte read (`lb`, so
 *           the cell is s8) and returns while that byte is nonzero. On the
 *           frame the gate byte reads zero it calls the shared helper at
 *           0x8015C088 and then stores 0x2D into the shared state entry byte at
 *           0x801448EB and zero into the shared byte at 0x801448EC, whose role
 *           is not established and stays address-anchored. Takes no arguments
 *           and returns nothing; the 0x18-byte frame exists only to hold the
 *           saved return address across that call (`sw ra,0x10(sp)`).
 *           Constant/variance evidence: the 0x50 bytes 0x801F5B50..0x801F5BA0
 *           have no byte-identical copy - a scan of every payload under
 *           out/binaries returned this span only - so the clone lever is
 *           declined and the body is written from these bytes. The nearest
 *           shape sibling is the exact lift
 *           setStatusFFFFAndStateEntryWhenGateClearWorld03Area14313_801F3B8C,
 *           which shares the unconditional 0xFFFF status store, the same gate
 *           test and the same state-entry and middle-byte store order but has
 *           no call and carries a third state store; this body keeps that
 *           statement order and writes its own literals (0x2D and zero) and
 *           drops the third store. Inside this payload the only reference to
 *           this address is the word at 0x801F64C8, entry 4 of the handler
 *           table based at 0x801F64B8 (no `jal`); entries 0..3 of that table
 *           are the exact lifts at 0x801F5A68, 0x801F5A98, 0x801F5AC8 and
 *           0x801F5B0C, so the handler role and the argument-free signature
 *           come from the table rather than from a caller.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateStatusClearGate(
    void) {
  D_801490A8 = 0xFFFFu;
  if (D_801490C7 == 0) {
    func_8015C088();
    D_801448EB = 0x2D;
    D_801448EC = 0;
  }
}
