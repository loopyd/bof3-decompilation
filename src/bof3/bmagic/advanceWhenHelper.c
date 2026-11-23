#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell. Declared as a named symbol so each
 * access is a lui plus offset load, which is how the original re-materializes
 * the cell address before the work-object store (lui $v1,0x1F80 /
 * lw $v1,0x44($v1)). */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind unknown */

/* Shared readiness helper, outside this overlay's image. It reports a readiness
 * byte, which the exact lifts of this target read as a non-zero ready flag.
 * Splat binds the address through this target's generated
 * undefined_funcs_auto.txt, so no target-map row is needed. */
u8 func_8014DAEC(void);

/* @source 0x801EFB90
 * @behavior Entry 13 of the overlay handler table that starts at 0x801EEC04,
 *           the word the header table stores at 0x801EEC38 and index 1 of the
 *           eight-word sub-run at 0x801EEC34 (0x801EFAE4, 0x801EFB90,
 *           0x801EFBD8, 0x801EFCC4, 0x801EFD7C, 0x801EFDE8, 0x801EFE84,
 *           0x801EFEFC) that
 *           dispatchWorkByte2HandlerThenInvokeSharedServiceWhenWorkBytes0And2Set
 *           BmagicMagic01500_801EFA28 copies into its frame and selects with
 *           unsigned work byte 2 of the scratchpad work object held in the
 *           pointer cell 0x1F800044. The payload holds that one table word and
 *           no jal or j word to this address, so the call site is that indirect
 *           dispatch. Per frame it calls the shared readiness helper 0x8014DAEC
 *           and tests the byte that helper returns: while the helper reports
 *           not-ready the handler does nothing, and once it reports ready the
 *           handler loads the work object through the pointer cell 0x1F800044
 *           and increments work byte 2, the handler index, with a byte-width add
 *           (lbu / addiu 1 / sb), which selects run entry 2 (0x801EFBD8). The
 *           0x18-byte frame only keeps $ra at 0x10 across the call. The
 *           0x48-byte body is byte-identical to the span at 0x801F014C of this
 *           same payload, proven by a payload cmp of the two spans, and the
 *           exact lift at 0x801F117C of bmagic magic008/00 supplied the
 *           statement order. Nothing is returned and no other state is read or
 *           written.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceWhenHelper(void) {
  if (func_8014DAEC() != 0u) {
    D_1F800044[2]++;
  }
}
