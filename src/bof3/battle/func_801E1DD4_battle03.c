#include "bof3/battle/battle03_internal.h"

/* @source 0x801E1DD4
 * @behavior Resets the current local work state: stores 2 in work byte +0x01
 * and clears bytes +0x02/+0x03/+0x04, clears the pending bit selected by work
 * byte +0x05, clears work flag 0x200, and clears work byte +0x119 when battle
 * flags 0x801462E8 bit 0x40 is clear.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801E1DD4(void) {
  D_1F800044->unk_01 = 2;
  D_1F800044->unk_02 = 0;
  D_1F800044->unk_03 = 0;
  D_1F800044->unk_04 = 0;
  clearPendingBit(D_1F800044->unk_05);
  BATTLE_LOCAL_WORD_124(BATTLE_LOCAL_WORK_PTR) &= 0xfffffdffu;
  if ((BATTLE_GLOBAL_HALF_62E8 & 0x40u) == 0u) {
    BATTLE_LOCAL_BYTE_119(BATTLE_LOCAL_WORK_PTR) = 0u;
  }
}
