#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell shared by the bmagic overlays. The
 * original loads the cell once (lui $v1,0x1F80 / lw $v1,0x44($v1)) before the
 * frame is created and reads both work bytes through that single value, so the
 * two accesses below are one group with no store between them and need no local
 * of their own. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared battle work record pointer in main RAM, outside this overlay's image.
 * This target's generated undefined_syms_auto.txt already binds the address
 * because the remaining assembly spans func_801EEC18 and func_801EF110 reach it
 * through a lui percent-hi / lw percent-lo pair, so the name needs no
 * target-map row. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

void func_800A9820(u8 slot);
void func_801E5988(void);

/* @source 0x801EF2E4
 * @behavior Step handler of the battle work object, entry 4 (last) of this
 *           overlay's five-entry code pointer run at 0x801EEC04
 *           (0x801EEFE4, 0x801EF110, 0x801EF190, 0x801EF1C8, 0x801EF2E4),
 *           whose only in-payload reader is the dispatcher
 *           dispatchWorkByte2HandlerBmagicMagic06303_801EEF74: it copies the
 *           run into its own frame and indexes it with work byte 2, so this
 *           entry is invoked with no arguments. The body reads the scratchpad
 *           work-object pointer cell at 0x1F800044 once, before the 0x18-byte
 *           frame is created, and tests the object's unsigned byte 0x0B; only
 *           when that byte is zero does it pass the object's unsigned byte 0x04
 *           to the main-executable helper at 0x800A9820, load the shared battle
 *           work record pointer at 0x801EB4E0, decrement that record's byte 0x0B
 *           by one at byte width (lbu 0xB / addiu -1 / sb 0xB, the stored value
 *           never tested) and invoke the shared helper at 0x801E5988, the store
 *           filling the call delay slot. The frame keeps only $ra at 0x10. It
 *           returns through the jr $ra at 0x801EF338 and writes no other state;
 *           the roles of work byte 0x0B, work byte 0x04 and the global record
 *           byte 0x0B are not established, so they stay offset-anchored. The
 *           92-byte span was byte-identical to emi/bmagic/magic146/03
 *           0x801EF2A0 when this body was written, and that twin was still an
 *           assembly span there, so it supplied no source; the statement order,
 *           cell spellings and the decrement-then-helper tail were taken from
 *           the siblings whose own gates measured exact when this body was
 *           written,
 *           dispatchWorkRecordByte1HandlerBmagicMagic06303_801EF340,
 *           incrementWorkByteAThenSetGlobalWorkByteBToFFAt80AndInvokeHelper
 *           BmagicMagic10803_801EF130 and
 *           decrementGlobalWorkByteBAndInvokeHelperBmagicMagic01200_801EF670.
 * @status exact
 * @match 100.00
 * @residual none
 */
void invokeHelperA9820WithWorkByte4WhenWorkByteBIsZeroThenDecrementGlobalWorkByteBAndInvokeHelperBmagicMagic06303_801EF2E4(void) {
  if (g_battle_work[0x0B] == 0) {
    func_800A9820(g_battle_work[0x04]);
    D_801EB4E0[0x0B]--;
    func_801E5988();
  }
}
