#include "bof3/battle/battle15_internal.h"

/* @source 0x800AD4D4
 * @behavior Publishes the grid offsets of work byte 0x0B relative to the
 *   D_80148628 pair into work halfwords 0x2E/0x30, then increments work byte 2
 *   once per entry of the active byte list that matches that byte.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * The volatile scratchpad work base is re-read for each published offset and
 * again on every loop iteration, so the two magic-multiply chains of the
 * publish step and the loaded base each stay independent. The iteration reload
 * is a separate local (not the publish base): sharing one local makes gcc reuse
 * the publish base register in the loop and permutes the index/count register
 * pair (measured 80.33% -> 100.00%). The loop-bound address is re-derived per
 * iteration, the exact sibling shape of func_800AD5C8.
 */
void func_800AD4D4(void)
{
  u8 *count;
  u8 *loop_count;
  u8 *work;
  u8 *current_work;
  u8 i;

  work = g_battle_work;
  *(u16 *)(work + 0x2E) =
      *(u16 *)&D_80148628 + ((u8)(work[0xB] % 6) * 30 + 0x13);
  *(u16 *)(work + 0x30) = D_8014862A + ((u8)(work[0xB] / 6) * 32 + 0x30);

  count = &D_801463C7;
  i = 0;
  if (*count != 0) {
    do {
      current_work = g_battle_work;
      if (D_801463C4[i] == current_work[0xB]) {
        current_work[2]++;
      }
      loop_count = &D_801463C7;
      i++;
    } while (i < *loop_count);
  }
}
