#include "internal.h"

/* @source 0x801E4ECC
 * @behavior Calls the enemy-ready helper unless the selected kind has bit 0x800,
 * then advances scratchpad work byte +0x09.
 */
void func_801E4ECC(void) {
  u16 index;

  index = BATTLE_GLOBAL_HALF_63C0;
  if (!(D_801CA71C[index].mask_00 & 0x800u)) {
    enemyReadyOrHelper2();
  }
  D_1F800044->pad_09[0]++;
}
