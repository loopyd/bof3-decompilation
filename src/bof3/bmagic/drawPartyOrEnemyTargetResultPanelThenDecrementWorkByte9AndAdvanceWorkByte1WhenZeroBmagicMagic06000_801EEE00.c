#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell at 0x1F800044, bound by this target's
 * map. The original re-materializes the cell address before each of the two
 * body accesses (lui $v1,0x1F80 / lw $v1,68($v1)), because the byte store
 * through the loaded pointer may alias the cell, so both statements stay on
 * the named cell instead of sharing one loaded base. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Battle target selector byte in main RAM, outside this overlay's image: it
 * chooses between the party and the non-party result path here exactly as it
 * does in the neighbouring step handler at 0x801EEE84. Splat already binds the
 * address through this target's generated undefined_syms_auto.txt, which
 * several still-asm spans reference. */
extern u8 D_80146384; /* @source 0x80146384 @kind unknown */

/* Result panel of a party battle target: draws the four-character name and the
 * ten static panel labels of this overlay. */
void drawWorkRecordNameAndBattleResultPanelLabelsBmagicMagic06000_801EF174(void);

/* Result panel of a non-party battle target, a source of this same target: it
 * scales (selector - 3) by the 0x118-byte local panel record stride and draws
 * that row's two numbers or its 0x5C placeholder cells. */
void drawEnemyTargetResultPanelNameNumbersAndValueRowsBmagicMagic06000_801EF2DC(
    void);

/* @source 0x801EEE00
 * @behavior Work-byte-1 handler of this overlay, index 2 of the five-entry code
 *           pointer run at 0x801EEC04 (0x801EECA4, 0x801EED68, this address,
 *           0x801EEE84 and 0x801EEF04, terminated by a zero word) that the
 *           dispatcher at 0x801EEC34 copies into its 0x30-byte frame and
 *           selects with the scratchpad work object byte at offset 0x1 scaled
 *           by four. It reads the selector byte D_80146384, before the 0x18-byte
 *           frame exists, and compares it against 3 unsigned (sltiu 3): below
 *           that bound it draws the party result panel through
 *           drawWorkRecordNameAndBattleResultPanelLabelsBmagicMagic06000_801EF174,
 *           any larger selector the non-party path through
 *           drawEnemyTargetResultPanelNameNumbersAndValueRowsBmagicMagic06000_801EF2DC,
 *           so exactly one arm runs. It then decrements the work object byte at
 *           offset 0x9 (lbu, addiu -1, sb through one loaded base) and, when the
 *           decremented byte is zero, advances the work object byte at offset
 *           0x1 by one from a fresh read of the pointer cell 0x1F800044. The
 *           0x18-byte frame keeps only ra at 0x10, nothing is returned and no
 *           other state is read or written.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawPartyOrEnemyTargetResultPanelThenDecrementWorkByte9AndAdvanceWorkByte1WhenZeroBmagicMagic06000_801EEE00(
    void) {
  if (D_80146384 < 3u) {
    drawWorkRecordNameAndBattleResultPanelLabelsBmagicMagic06000_801EF174();
  } else {
    drawEnemyTargetResultPanelNameNumbersAndValueRowsBmagicMagic06000_801EF2DC();
  }
  if (--g_battle_work[9] == 0) {
    g_battle_work[1]++;
  }
}
