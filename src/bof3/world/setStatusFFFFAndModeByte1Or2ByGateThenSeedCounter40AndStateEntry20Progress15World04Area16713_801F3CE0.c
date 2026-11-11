#include "bof3/bof3.h"

/* Shared status halfword of the main executable, outside this overlay's
 * payload. The setStatusFFFFAndStateEntry handlers of the world family store
 * 0xFFFF here on every path and this span writes the same constant. */
extern u16 D_801490A8; /* @source 0x801490A8 @kind data */

/* Shared gate byte of the main executable, outside this overlay's payload.
 * This span selects its mode value on whether the byte is zero and skips the
 * rest of its work, keeping only the status store, when the byte reads 2. */
extern s8 D_801490C7; /* @source 0x801490C7 @kind data */

/* Shared mode byte of the main executable, outside this overlay's payload. The
 * scenario overlays name 0x80146866 the shared mode byte and branch on its
 * small values; this span stores the 1 or 2 the gate byte selects. */
extern u8 D_80146866; /* @source 0x80146866 @kind data */

/* Shared counter byte of the main executable, outside this overlay's payload.
 * The world handler at 0x801F394C reads 0x80146867 as the count it waits on
 * and increments, so the 0x40 stored here seeds that count. */
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

/* @source 0x801F3CE0
 * @behavior Stores 0xFFFF into the shared status halfword at 0x801490A8 on
 *           every path. When the shared gate byte at 0x801490C7 reads 2 it
 *           returns straight from there; otherwise it stores 1 in the shared
 *           mode byte at 0x80146866 when the gate byte is zero and 2 when it
 *           is not, runs the shared front-end startup helper func_8015C088,
 *           and then seeds the shared counter byte at 0x80146867 with 0x40,
 *           the shared state entry byte at 0x801448EB with 0x20 and the shared
 *           progress byte at 0x801448EC with 0x15. The gate is read with `lb`
 *           (so the cell is s8) and that load is hoisted above the status
 *           store; the two mode values merge into one `sb` whose value
 *           register carries 2 on the taken side of `bnez` and 1 on the
 *           fallthrough, and the `beq` on 2 jumps to the shared epilogue. The
 *           0x18-byte frame only keeps $ra across the single call, and the
 *           call delay slot stays a nop because the helper takes no arguments.
 *           Every byte store materialises its 0x8014 page in $at on its own and
 *           every stored literal needs its own `addiu`. Takes no arguments and
 *           returns nothing.
 *
 * Name evidence: 0x801490A8 is the shared status halfword and 0x801490C7 the
 * shared gate byte of the setStatusFFFFAndStateEntry family, as the in-scope
 * exact siblings setStatusFFFFAndStateEntry3BProgress0SetSharedFlagBit8FWhenGateClearWorld04Area16813_801F4920
 * and setStatusFFFFAndStateEntry2BSubstate1WhenGateClearWorld04Area18813_801F4670
 * record them, and 0x801448EB and 0x801448EC are that family's state entry and
 * progress bytes. 0x80146866 is the shared mode byte the scenario overlays
 * branch on (runSecondaryStateSequenceForPrimaryState17ScenarioScena0600_801FC718,
 * func_801F92DC, func_801FB744) and 0x80146867 is the shared counter byte the
 * world handler func_801F394C of target emi/world03/area027/13 waits on and
 * increments, so the 0x40 written here is a seed for that count.
 * func_8015C088 is the shared front-end startup helper the neighbouring
 * handlers of this target run. The payload names this body only from word 0 of
 * the overlay handler table at 0x801F6830, which no `jal` reaches, so its
 * argument roles stay structural and none are taken. The role of the gate
 * value 2, of the mode values 1 and 2 and of the counter seed 0x40 is not
 * established beyond this span, so the name states the stored constants and
 * leaves the meaning to the gate.
 *
 * Shape evidence: the two payload spans that follow this one in the same
 * handler table, 0x801F3D54 (0x80 bytes) and 0x801F3DD4 (0x80 bytes), both
 * still `asm` rows, are this body word for word from the frame creation to the
 * state entry store and differ only in the two mode literals (0 and 2 there,
 * 0 and 1 there, against 1 and 2 here), in the progress literal (0x14 there,
 * 0x15 here) and in branch topology: with 1 and 2 this compiler merges the two
 * mode stores into one `sb` and writes an `if`/`else` for the zero case alone,
 * while a zero literal stays a separate `sb $zero` with a `j` around the else
 * store. Statement order, operand forms and declaration style are taken from
 * those spans and from the exact sibling 0x801F4920.
 *
 * Clone declined: a scan of every extracted payload under out/binaries for this
 * 0x74-byte body returned only this span's own offset in its own payload, so no
 * byte-identical twin exists anywhere and the table neighbours above are shape
 * evidence rather than a clone.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setStatusFFFFAndModeByte1Or2ByGateThenSeedCounter40AndStateEntry20Progress15World04Area16713_801F3CE0(
    void) {
  D_801490A8 = 0xFFFFu;
  if (D_801490C7 == 2) {
    return;
  }
  if (D_801490C7 == 0) {
    D_80146866 = 1;
  } else {
    D_80146866 = 2;
  }
  func_8015C088();
  D_80146867 = 0x40;
  D_801448EB = 0x20;
  D_801448EC = 0x15;
}
