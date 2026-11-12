#include "bof3/bof3.h"

void func_801E5988(void);

/* Shared battle work record pointer in main RAM, outside this overlay's image.
 * This target's generated undefined-syms map already binds the address because
 * the overlay's remaining assembly loads it, so the name needs no map row. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* @source 0x801F0300
 * @behavior Reads the shared battle work record pointer at 0x801EB4E0 once,
 *           decrements that record's byte 0xB by one at byte width (lbu 0xB /
 *           addiu -1 / sb 0xB) and then invokes the shared helper at 0x801E5988
 *           unconditionally, the store filling the call's delay slot. The
 *           pointer load precedes frame creation because the decrement needs no
 *           stack; the 0x18-byte frame only keeps $ra across the call. Takes no
 *           arguments, returns nothing and reads or writes no other state. The
 *           address is entry 24 of this overlay's 39-word code-pointer run, the
 *           word the tabledata segment stores at 0x801EEC64; the run's only
 *           in-payload reader, dispatchWorkByte1HandlerBmagicMagic01500_801EECA0,
 *           copies its first four words, and no jal word in the payload targets
 *           0x801F0300, so the caller that selects this entry lies outside the
 *           overlay. The 52 bytes are identical to the exact siblings
 *           emi/bmagic/magic012/00@0x801EF670,
 *           emi/bmagic/magic038/03@0x801EF884 and
 *           emi/bmagic/magic138/03@0x801EF6F8, which name the same cell and the
 *           same helper, so no relocation differs.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementGlobalWorkByteBAndInvokeHelperBmagicMagic01500_801F0300(void) {
  D_801EB4E0[0xB]--;
  func_801E5988();
}
