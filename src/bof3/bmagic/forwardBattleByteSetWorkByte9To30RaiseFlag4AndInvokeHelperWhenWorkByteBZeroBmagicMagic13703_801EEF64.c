#include "bof3/bof3.h"

void func_800A9820(u8 arg0);
void func_801E5988(void);

/* Scratchpad work-object pointer cell at 0x1F800044, bound by this target's map
 * row g_battle_work, the name its other exact lifts use. The original takes the
 * cell once, before the frame is created, for the work-byte-0xB test, and then
 * re-reads it after the first call, because that callee may repoint the cell. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared battle byte at 0x80146384: the zero-extended battle target byte that
 * func_800A9820 consumes. This target's generated undefined_syms_auto.txt
 * already binds the address, and the byte lives outside this overlay, so it
 * keeps its address-qualified name. */
extern u8 D_80146384; /* @source 0x80146384 @kind unknown */

/* Shared battle global halfword at 0x801462E8, bound the same way by this
 * target's generated undefined_syms_auto.txt. Bit 0x4 is raised through an
 * unsigned halfword load, an ori and a halfword store that re-materialises the
 * address into $at. It lives outside this overlay, so it keeps its
 * address-qualified name. */
extern volatile u16 D_801462E8; /* @source 0x801462E8 @kind unknown */

/* @source 0x801EEF64
 * @behavior Reads the scratchpad work-object pointer cell at 0x1F800044 once,
 *           before its frame is created, and returns immediately when the work
 *           object's byte at offset 0xB is non-zero (lbu, bnez with an empty
 *           delay slot). Otherwise it loads the shared battle byte at
 *           0x80146384 unsigned and passes it zero-extended to the shared
 *           helper func_800A9820, re-reads the pointer cell and stores the
 *           countdown 0x1E into work byte 0x9 at byte width, raises bit 0x4 of
 *           the shared battle global halfword 0x801462E8 (unsigned halfword
 *           load, ori, halfword store through $at) and then invokes the shared
 *           helper func_801E5988. It takes no arguments, returns nothing and
 *           writes no other state; the 0x18-byte frame only keeps $ra at 0x10
 *           across the two calls. The pointer cell is re-read after the first
 *           call rather than kept in a register, so the store after it observes
 *           a cell the callee may have repointed.
 *           Reached through entry 2 (0x801EEF64) of the three-word code pointer
 *           run at 0x801EEC04, which the dispatcher at 0x801EEC1C reads and
 *           indexes by work byte 0x1, so no jal word in the payload names it:
 *           an overlay dispatch entry, not a called function. Work-object byte
 *           0xB, the 0x1E countdown and the shared battle byte stay
 *           offset-anchored.
 *           Shape: this 112-byte span equals emi/bmagic/magic041/03@0x801EF130,
 *           the exact lift forwardBattleByteThenRaiseFlag4AndInvokeHelper
 *           WhenWorkByteBZero, word for word except for four instructions
 *           inserted before the shared-halfword sequence (lui, lw of
 *           0x1F800044, addiu $v0,$zero,0x1E, sb to offset 9) and the single
 *           forward branch whose displacement grows by those four words; every
 *           other word, including both jal immediates and the branch's empty
 *           delay slot, is equal. A find of the 112 payload bytes across
 *           out/binaries returned only this target's own span, so no
 *           byte-identical twin exists and the clone lever was declined; the
 *           statement order, operand forms and declaration style are
 *           transcribed from that surviving-shape sibling, with this span's own
 *           literals and the inserted store written in the position the bytes
 *           show.
 * @status exact
 * @match 100.00
 * @residual none
 */
void forwardBattleByteSetWorkByte9To30RaiseFlag4AndInvokeHelperWhenWorkByteBZeroBmagicMagic13703_801EEF64(void) {
  u8 *work = g_battle_work;

  if (work[0xB] == 0) {
    func_800A9820(D_80146384);
    g_battle_work[9] = 0x1E;
    D_801462E8 |= 4;
    func_801E5988();
  }
}
