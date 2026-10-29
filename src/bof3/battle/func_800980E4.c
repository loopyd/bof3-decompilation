#include "bof3/battle/battle15_internal.h"

/* @source 0x800980E4
 * NOTE: byte-match blocked by the `jr ra` epilogue scheduler reorg: the repo
 * build's default `-fschedule-insns` hoists the selection-slot store out of
 * the `jal` delay slot. The original battle/15 object used `-fno-schedule-insns`;
 * restoring that per-target profile makes this match.
 * @behavior Stores the low byte of func_801DB5CC(3) through the pointer at 0x801EB4D8,
 * then calls func_801DE94C(2, 0). Sets byte 0x801462EF to 1, clears halfword
 * 0x80145AC8 and increments byte 0x801462E3 modulo 256.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_800980E4(void) {
  u8  result;
  u8* game_ram;

  result = func_801DB5CC(3);
  *BATTLE_ACTIVE_SELECTION_SLOT_PTR = result;

  func_801DE94C(2, 0);

  D_801462EF = 1;
  D_80145AC8 = 0;
  game_ram = (u8*)BATTLE_GAME_RAM_BASE;
  game_ram[0x62E3] = (u8)(game_ram[0x62E3] + 1u);
}
