#include "bof3/bof3.h"

/* Shared status halfword of the main executable, outside this overlay's
 * payload. The setStatusFFFFAndStateEntry handlers of the world family store
 * 0xFFFF here on every path and this span writes the same constant. */
extern u16 D_801490A8; /* @source 0x801490A8 @kind data */

/* Shared gate byte of the main executable, outside this overlay's payload.
 * This span runs its seed block only while the byte reads zero and skips
 * straight to the epilogue, keeping only the status store, on every other
 * value. */
extern s8 D_801490C7; /* @source 0x801490C7 @kind data */

/* Shared counter byte of the main executable, outside this overlay's payload.
 * The world handler func_801F394C of target emi/world03/area027/13 reads
 * 0x80146867 as the count it waits on and increments, so the 0x60 stored here
 * seeds that count. */
extern u8 D_80146867; /* @source 0x80146867 @kind data */

/* State entry byte of the shared front-end tuple in the main executable,
 * outside this overlay's payload. The setStatusFFFFAndStateEntry handlers of
 * the world family request their entry through it. */
extern u8 D_801448EB; /* @source 0x801448EB @kind data */

/* Progress byte of the same shared front-end tuple, written beside the entry
 * byte by every handler of that family. */
extern u8 D_801448EC; /* @source 0x801448EC @kind data */

/* Shared front-end startup helper of the main executable, outside this
 * overlay's payload, also reached from the neighbouring handlers of this
 * target. */
extern void func_8015C088(void); /* @source 0x8015C088 */

/* @source 0x801F3E54
 * @behavior Entry 3 of the 25-entry overlay handler table at 0x801F6830 of
 *           this payload, whose entry 0 is the exact lift
 *           stateStatusGateSeed
 *           and whose entry 2 is the exact lift
 *           setStatusFFFFAndModeByte0Or1ByGateThenSeedCounter40AndStateEntry20Progress14World04Area16713_801F3DD4.
 *           Stores 0xFFFF into the shared status halfword at 0x801490A8 on
 *           every path and returns straight from there whenever the shared
 *           gate byte at 0x801490C7 reads nonzero; only a zero gate byte runs
 *           the shared front-end startup helper func_8015C088 and then seeds
 *           the shared counter byte at 0x80146867 with 0x60, the shared state
 *           entry byte at 0x801448EB with 0x20 and the shared progress byte at
 *           0x801448EC with 0x0A. Unlike its two sibling handlers this span
 *           stores no mode value into 0x80146866. The gate is read with `lb`
 *           (so the cell is s8) and that load is hoisted above the status
 *           store; the zero arm keeps its own `sb` and the `bnez` on the gate
 *           jumps to the shared epilogue. The 0x18-byte frame only keeps $ra
 *           across the single call, and the call delay slot stays a nop
 *           because the helper takes no arguments. Every byte store
 *           materialises its 0x8014 page in $at on its own and every stored
 *           literal needs its own `addiu`. Takes no arguments and returns
 *           nothing.
 *
 * Name evidence: 0x801490A8 is the shared status halfword and 0x801490C7 the
 * shared gate byte of the setStatusFFFFAndStateEntry family, as the in-scope
 * exact siblings setStatusFFFFAndStateEntry3BProgress0SetSharedFlagBit8FWhenGateClearWorld04Area16813_801F4920
 * and setStatusFFFFAndStateEntry2BSubstate1WhenGateClearWorld04Area18813_801F4670
 * record them, and 0x801448EB and 0x801448EC are that family's state entry and
 * progress bytes. 0x80146867 is the shared counter byte the world handler
 * func_801F394C of target emi/world03/area027/13 waits on and increments, so
 * the 0x60 written here is a seed for that count. func_8015C088 is the shared
 * front-end startup helper the neighbouring handlers of this target run. The
 * payload names this body only from word 3 of the overlay handler table at
 * 0x801F6830, which no `jal` reaches, so its argument roles stay structural
 * and none are taken. The role of the gate value zero and of the counter seed
 * 0x60 is not established beyond this span, so the name states the stored
 * constants and leaves the meaning to the gate.
 *
 * Shape evidence: the body is transcribed from the exact lift
 * setStatusFFFFAndModeByte0Or1ByGateThenSeedCounter40AndStateEntry20Progress14World04Area16713_801F3DD4
 * of word 2 of the same handler table, whose statement order, operand forms and
 * declaration style are followed here: the shared status store, the single
 * zero test of the same gate byte, the helper call and the three-byte seed run
 * keep that order. This span drops that sibling's mode store, its test on the
 * gate value 2 and its branch, and writes its own literals (counter 0x60 and
 * progress 0x0A there 0x40 and 0x14), which is why the status store's `$ra`
 * spill lands one instruction earlier here, in the slot the sibling spends on
 * the 0x2 comparison. The next span 0x801F3EB4, word 4 of the same table, was
 * compared with this one over its full 0x60 bytes: `cmp` of the two payload
 * ranges reported no differing byte, so the two are one body, and that span
 * takes this body as its own exact lift
 * setStatusFFFFAndSeedCounter60AndStateEntry20Progress0AWhenGateByteZeroWorld04Area16713_801F3EB4.
 *
 * Clone declined: the byte-identical twin 0x801F3EB4 sits in this same target,
 * where `bin/harness lift clone` refuses with "clone target is the
 * representative's own target", and a payload scan of all 398 shipped
 * binaries found no other copy of this span. The body was seeded from the
 * exact sibling 0x801F3DD4 instead of cloned.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateStatusSeedGate(
    void) {
  D_801490A8 = 0xFFFFu;
  if (D_801490C7 == 0) {
    func_8015C088();
    D_80146867 = 0x60;
    D_801448EB = 0x20;
    D_801448EC = 0x0A;
  }
}
