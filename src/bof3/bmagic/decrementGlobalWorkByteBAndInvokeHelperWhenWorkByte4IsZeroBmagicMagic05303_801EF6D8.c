#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell shared by the battle and bmagic overlays.
 * Every access here is byte-addressed, so the byte view is all this body needs,
 * and the name is the one this target's exact lift
 * clearWorkBytesBmagicMagic05303_801F04F8 already uses for the same cell. The
 * original materializes the cell once (lui $v0,0x1F80 / lw $v0,0x44($v0))
 * before the frame is created and reads work byte 0x04 through that value, so
 * the test below is one access group with no store interleaved. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared battle work record pointer in main RAM, outside this overlay's image.
 * This target's generated undefined_syms_auto.txt already binds the address
 * because the remaining assembly spans reach it through a lui percent-hi /
 * lw percent-lo pair, so the name needs no target-map row, and the access here
 * is byte-addressed. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Shared overlay-family helper in the main executable, outside this overlay's
 * image. Its role is not established, so it stays address-anchored. */
void func_801E5988(void);

/* @source 0x801EF6D8
 * @behavior Handler at entry index 5 of the ten-pointer code run
 *           0x801F0550..0x801F0574 (0x801EF0A8, 0x801EF174, 0x801EF420,
 *           0x801EF5C0, 0x801EF674, 0x801EF6D8, 0x801EFD94, 0x801EFE1C,
 *           0x801EFF24, 0x801F0018); the word at 0x801F0564 is the only word in
 *           the payload that names this address and no jal or j reaches it, and
 *           that table's only in-payload reader is the dispatcher
 *           dispatchStateByte1Table0550BmagicMagic05303_801EF064, which indexes
 *           the run with the work record's unsigned byte +1, so this body is
 *           invoked with no arguments. It reads the scratchpad work-object
 *           pointer cell at 0x1F800044 once, before the 0x18-byte frame is
 *           created, and tests the work object's unsigned byte 0x04 for zero
 *           with lbu and bnez on the zero-extended byte (no andi). Only when
 *           that byte is zero does it load the shared battle work record
 *           pointer at 0x801EB4E0, decrement that record's byte 0x0B by one at
 *           byte width (lbu 0xB / addiu -1 / sb 0xB) and invoke the shared
 *           helper at 0x801E5988, the store filling the call delay slot and the
 *           stored value never tested. The frame keeps only $ra at 0x10, the
 *           body returns through the jr $ra at 0x801EF720 and reads or writes no
 *           other state. The roles of work byte 0x04 and of record byte 0x0B are
 *           not established, so both stay offset-anchored. The 0x50-byte span
 *           has no byte-identical window in any extracted payload under
 *           out/binaries, so the clone lever offered no source, and no window
 *           matched with this body's branch, address and call words masked
 *           either. The statement order, operand forms, offset spelling and
 *           declaration style follow the exact siblings
 *           decrementGlobalWorkByteBAndInvokeHelperA9820WithWorkByte4When
 *           WorkByteBIsZeroThenInvokeHelperBmagicMagic07303_801EF6B8 (whose
 *           guard, cell load before frame creation and decrement-then-helper
 *           tail match this span) and
 *           invokeHelperA9820WithWorkByte4WhenWorkByteBIsZeroThenDecrement
 *           GlobalWorkByteBAndInvokeHelperBmagicMagic06303_801EF2E4, both of
 *           which measured exact when this body was written; each of them also
 *           passes work byte 0x04 or decrements record byte 0x0B, which this
 *           span does not, so only the shared statements were taken from them.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementGlobalWorkByteBAndInvokeHelperWhenWorkByte4IsZeroBmagicMagic05303_801EF6D8(void) {
  if (g_battle_work[0x04] == 0) {
    D_801EB4E0[0x0B]--;
    func_801E5988();
  }
}
