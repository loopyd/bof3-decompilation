#include "bof3/bof3.h"

extern void func_801E5988(void); /* @source 0x801E5988 @kind function */

/* Shared battle global halfword at 0x801462E8. It lives outside this overlay,
 * in the main executable every battle overlay runs against, so it keeps its
 * address-qualified name; this target's generated undefined_syms_auto.txt binds
 * the address, so no target-map row is added. */
extern volatile u16 D_801462E8; /* @source 0x801462E8 @kind unknown */

/* @source 0x801EEE94
 * @behavior Raises bit 0x4 of the shared battle global halfword at 0x801462E8
 *           and then invokes the helper at 0x801E5988 with no arguments. Its
 *           address is the third word (index 2) of the three-word code pointer
 *           run in this payload's leading data at 0x801EEC04, whose words are
 *           0x801EEC70, 0x801EEE38 and 0x801EEE94; the payload's own dispatcher
 *           callWorkHandlerTableec04 copies
 *           those three words into its 0x28-byte frame and calls the entry
 *           selected by the scratchpad work object byte at offset 0x1 scaled by
 *           four, which is how this body is reached. The original materializes
 *           the halfword address once in a register (unsigned halfword load,
 *           ori with 0x4, halfword store through that one register, the store
 *           filling the call delay slot) and keeps it live across the call, so
 *           the read-modify-write is written through a local pointer; the
 *           0x18-byte frame only keeps the return address at 0x10 across the
 *           helper call. Nothing is returned and no other state is read or
 *           written. A payload compare of this 0x34-byte span against
 *           0x801EF17C of emi/bmagic/magic056/03 came out equal in all 52
 *           bytes, and that sibling had measured exact, so this body
 *           transcribes the sibling
 *           raiseFlag4AndInvokeHelperBmagicMagic05603_801EF17C unchanged, with
 *           this payload's own callee binding and symbol rows.
 * @status partial
 * @match 85.71
 * @residual Preserving volatile access keeps the halfword store before the call and adds four bytes.
 */
void raiseFlag4InvokeHelper2(void) {
    volatile u16 *flags = &D_801462E8;

    *flags |= 4;
    func_801E5988();
}
