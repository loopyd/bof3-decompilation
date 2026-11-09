#include "bof3/bof3.h"

void func_801E5988(void);

/* Scratchpad work-object pointer cell 0x1F800044, bound by this target's map row
 * g_battle_work. Declared as the named cell this target's lifted sources use: the
 * symbolic address folds into each load (lui $vN,0x1F80 / lw $vN,0x44($vN)), and
 * the original re-reads the cell before the second access group because the store
 * through the pointer the cell supplied may alias the cell. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared battle work record pointer in main RAM, outside this overlay's image.
 * splat binds 0x801EB4E0 through this target's generated undefined_syms_auto.txt,
 * so no target-map row is needed. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* @source 0x801EFFB0
 * @behavior Per-frame step of the work object published at the scratchpad pointer
 *           cell 0x1F800044. It increments unsigned object byte 0xA at byte width
 *           (lbu $v0,10($v1) / addiu $v0,$v0,1 / sb $v0,10($v1)), then re-reads
 *           the pointer cell with its own lui $v1,1F80 / lw $v1,68($v1) pair and
 *           decrements unsigned object byte 0x9 at byte width (lbu / addiu -1 /
 *           sb), masking the value it just stored with andi 0xFF so the truncated
 *           register - not a reload - supplies the compared value. Only when that
 *           byte reaches 0x4 does it read the shared battle work record pointer at
 *           0x801EB4E0, subtract one from that record's byte 0xB at byte width
 *           and invoke the shared helper at 0x801E5988 in a jal whose delay slot
 *           holds the sb of byte 0xB. The 0x18-byte frame exists only to keep $ra
 *           across that call and is created after the first pointer load, which
 *           needs no stack. Both cell loads are required: the byte 0xA store goes
 *           through the pointer the cell supplied, so it may alias the cell. Takes
 *           no arguments, returns nothing and advances no work byte 2.
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementWorkByteAThenDecrementWorkByte9AndDecrementGlobalWorkByteBAndInvokeHelperWhenWorkByte9Is4BmagicMagic07903_801EFFB0(void)
{
  g_battle_work[0xA]++;
  if (--g_battle_work[0x9] == 0x4) {
    D_801EB4E0[0x0B] -= 1;
    func_801E5988();
  }
}
