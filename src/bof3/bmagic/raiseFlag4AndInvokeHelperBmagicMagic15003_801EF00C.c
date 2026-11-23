#include "bof3/bof3.h"

void func_801E5988(void);

/* Shared battle global halfword at 0x801462E8. It lies outside this payload, in
 * the main executable every battle overlay runs against, so it keeps its
 * address-qualified name; this target's generated undefined_syms_auto.txt
 * already binds the address, so no target-map row is added. It is declared
 * unsigned here because the original loads and stores it as a halfword. */
extern volatile u16 D_801462E8; /* @source 0x801462E8 @kind unknown */

/* @source 0x801EF00C
 * @behavior Raises bit 0x4 of the shared battle global halfword at 0x801462E8
 *           with a halfword read-modify-write and then invokes the helper at
 *           0x801E5988 with no arguments, unconditionally. This span is the
 *           seventh word (index 6) of the seven-word code pointer run in this
 *           payload's leading data at 0x801EEC04, whose words are 0x801EECA4,
 *           0x801EED60, 0x801EEDCC, 0x801EEE24, 0x801EEEC4, 0x801EEF80 and
 *           0x801EF00C; the payload's own dispatcher
 *           dispatchWorkByte1HandlerBmagicMagic15003_801EEC20 copies those
 *           words into its 0x38-byte frame and calls the one selected by the
 *           scratchpad work-object byte at offset 1, so no jal reaches this
 *           address and the table word is its only reference in the payload.
 *           The original materializes the halfword address once in a register
 *           (unsigned halfword load, ori with 0x4, halfword store through that
 *           one register, the store filling the call delay slot) and keeps it
 *           live across the call, so the read-modify-write is written through a
 *           local pointer. The 0x18-byte frame only keeps the return address at
 *           0x10 across the helper call. Nothing is returned, no work-object
 *           byte is touched and no other cell is read or written.
 * @status partial
 * @match 85.71
 * @residual Preserving volatile access keeps the halfword store before the helper call instead of its delay slot, adding one instruction.
 */
void raiseFlag4AndInvokeHelperBmagicMagic15003_801EF00C(void) {
  volatile u16 *flags = &D_801462E8;

  *flags |= 4;
  func_801E5988();
}
