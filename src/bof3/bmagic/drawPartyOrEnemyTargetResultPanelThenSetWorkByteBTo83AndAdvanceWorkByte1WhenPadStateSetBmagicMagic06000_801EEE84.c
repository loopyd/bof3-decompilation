#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell at 0x1F800044, bound by this target's
 * map. The original re-materializes the cell address before each of the two
 * body accesses (lui $v1,0x1F80 / lw $v1,68($v1)), because the byte store
 * through the loaded pointer may alias the cell, so both statements stay on
 * the named cell instead of sharing one loaded base. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Battle target selector byte in main RAM, outside this overlay's image, the
 * same cell the exact sibling
 * drawWorkRecordNameAndBattleResultPanelLabelsBmagicMagic06000_801EF174 reads
 * when it selects the 0x140-byte work record of the target being reported on.
 * Here it only chooses between the party and the non-party result path. Splat
 * already binds the address through this target's generated
 * undefined_syms_auto.txt, which several still-asm spans reference. */
extern u8 D_80146384; /* @source 0x80146384 @kind unknown */

/* Shared pressed-pad-state halfword in main RAM at 0x80145AA8, the cell the
 * exact shop00/ui lifts test as `D_80145AA8 != 0` after a frame. This overlay
 * is the only user in its own payload, so the target map row binds the address
 * once the span moves to C. */
extern u16 D_80145AA8; /* @source 0x80145AA8 @kind unknown */

/* Result panel of a party battle target: draws the four-character name and the
 * ten static panel labels of this overlay, lifted exact in this same target. */
void drawWorkRecordNameAndBattleResultPanelLabelsBmagicMagic06000_801EF174(void);

/* Result path of a non-party battle target, still an asm sibling of this
 * target: it scales (selector - 3) by the 0x118-byte enemy work-record stride
 * and, when that row's byte at +0xB is set, draws through func_801EF924 at the
 * same (0x70, 0x30) pen position this file's party arm uses. */
void func_801EF2DC(void);

/* @source 0x801EEE84
 * @behavior Result-panel step of the current battle target: reads the selector
 *           byte D_80146384 and, while it is below 3 (sltiu 3, so an unsigned
 *           compare), draws the party result panel through
 *           drawWorkRecordNameAndBattleResultPanelLabelsBmagicMagic06000_801EF174;
 *           any larger selector takes the non-party path through func_801EF2DC
 *           instead. The two arms are exclusive, and the value 0x83 is
 *           materialized in the compare's branch delay slot only to feed the
 *           next block. When the shared pressed-pad-state halfword D_80145AA8
 *           is nonzero it then marks the scratchpad work-object byte at offset
 *           0xB with 0x83 and advances that object's byte at offset 0x1 by one
 *           (lbu, addiu, sb). Both writes come from two separate reads of the
 *           scratchpad pointer cell 0x1F800044. Frame 0x18 holding $ra, no
 *           arguments, no return value and no other state touched. A byte scan
 *           of the extracted EMI payload set for this 128-byte span finds it
 *           only in this target's binary, so there was no lifted twin to clone.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawPartyOrEnemyTargetResultPanelThenSetWorkByteBTo83AndAdvanceWorkByte1WhenPadStateSetBmagicMagic06000_801EEE84(
    void) {
  if (D_80146384 < 3u) {
    drawWorkRecordNameAndBattleResultPanelLabelsBmagicMagic06000_801EF174();
  } else {
    func_801EF2DC();
  }
  if (D_80145AA8 != 0) {
    g_battle_work[0xB] = 0x83;
    g_battle_work[1]++;
  }
}
