#include "bof3/bof3.h"

void func_800A9820(u8 arg0);
void func_801E5988(void);

/* Scratchpad work-object pointer cell at 0x1F800044, bound by this target's map
 * row g_battle_work, the name the lifted siblings on this target use. The
 * original takes the cell once, before the frame is created, and the symbolic
 * address lets the assembler fold it into the load. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared battle byte at 0x80146384: the zero-extended battle target byte that
 * func_800A9820 consumes. This target's generated undefined_syms_auto.txt
 * already binds the address, and the byte lives outside this overlay, so it
 * keeps its address-qualified name. */
extern u8 D_80146384; /* @source 0x80146384 @kind unknown */

/* Shared battle global halfword at 0x801462E8, bound the same way by this
 * target's generated undefined_syms_auto.txt. Bit 0x4 is raised through an
 * unsigned halfword load, an ori and a halfword store. It lives outside this
 * overlay, so it keeps its address-qualified name. */
extern volatile u16 D_801462E8; /* @source 0x801462E8 @kind unknown */

/* @source 0x801EF130
 * @behavior Reads the scratchpad work-object pointer cell at 0x1F800044 once
 *           and returns immediately when the work object's byte at offset 0xB
 *           is non-zero. Otherwise it loads the shared battle byte at
 *           0x80146384 unsigned and passes it to the shared helper
 *           func_800A9820, raises bit 0x4 of the shared battle global halfword
 *           0x801462E8 (unsigned halfword load, ori, halfword store) and then
 *           invokes the shared helper func_801E5988. It takes no arguments,
 *           returns nothing and writes no other state; the 0x18-byte frame only
 *           keeps $ra at 0x10 across the two calls. The pointer cell is loaded
 *           before that frame is created, and the halfword store re-materialises
 *           its address into $at rather than keeping one base register live
 *           across the call. Reached through word 5 of the overlay code pointer
 *           run at 0x801EEC04, which the dispatcher entry copies and indexes, so
 *           no jal word names it. Work-object byte 0xB and the shared battle
 *           byte stay offset-anchored.
 *           Shape: the 96-byte span is byte-identical to
 *           emi/bmagic/magic106/03@0x801EEE80, which is lifted exact as
 *           forwardBattleByteThenRaiseFlag4AndInvokeHelperWhenWorkByteBZero,
 *           and to the still-unlifted copy at emi/bmagic/magic222/03@0x801EEE80
 *           (both compared with cmp over the 96 payload bytes). This body is
 *           that sibling's transcription: its statement order, operand forms
 *           and declaration style carry over, and this target's own map binds
 *           the scratchpad cell as g_battle_work.
 * @status exact
 * @match 100.00
 * @residual none
 */
void forwardBattleRaiseZero(void) {
  u8 *work = g_battle_work;

  if (work[0xB] == 0) {
    func_800A9820(D_80146384);
    D_801462E8 |= 4;
    func_801E5988();
  }
}
