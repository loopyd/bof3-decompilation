#include "bof3/bof3.h"

void func_801E5988(void);

/* Shared battle global halfword at 0x801462E8. The sibling battle lifts bind the
 * same halfword; it lives outside this overlay, so it keeps its
 * address-qualified name. Which battle state bit 0x4 records is not established
 * by this span, so the cell stays offset-anchored. */
extern volatile u16 D_801462E8; /* @source 0x801462E8 @kind unknown */

/* @source 0x801EF31C
 * @behavior Fifth entry, index 4, of the six-entry code pointer run at
 *           0x801EEC04 (0x801EECC0, 0x801EF0F8, 0x801EF234, 0x801EF2A8,
 *           0x801EF31C, 0x801EF350), which
 *           dispatchWorkByte1HandlerBmagicMagic02003_801EEC48 copies into its
 *           own 0x30-byte frame and indexes with work-object unsigned byte 1, so
 *           this body runs with no arguments and returns no value. It raises bit
 *           0x4 of the shared battle global halfword at 0x801462E8 and then
 *           invokes the shared helper at 0x801E5988. The original materializes
 *           the halfword address once into $v1 and keeps that one register live
 *           across the call, so the read-modify-write is written through a local
 *           pointer: lhu 0x0($v1), the R3000 load-delay nop, ori 0x4, the jal to
 *           0x801E5988, and the sh 0x0($v1) in that call's delay slot. The
 *           0x18-byte frame keeps only $ra at 0x10, every remaining delay slot in
 *           the span is nop, and nothing is returned. Nothing else is read or
 *           written. The 0x34-byte body is byte-identical to the raise-and-invoke
 *           entry at 0x801EF1CC of emi/bmagic/magic015/00, whose exact lift
 *           supplies this shape, and to the four such entries of
 *           emi/bmagic/magic008/00 at 0x801EEFDC, 0x801EF728, 0x801EFA40 and
 *           0x801EFCFC, so the clone lever applied; only the map name is
 *           target-qualified. The span opens with addiu $sp,-0x18 and closes in
 *           jr $ra through its own frame, so the data lever did not apply, and it
 *           sits between the reviewed boundaries at 0x801EF2A8, whose span ends
 *           at this address, and 0x801EF350, so the carve lever did not apply
 *           either.
 * @status exact
 * @match 100.00
 * @residual none
 */
void raiseFlag4AndInvokeHelperBmagicMagic02003_801EF31C(void) {
  u16 *flags = (u16 *)&D_801462E8;

  *flags |= 4;
  func_801E5988();
}
