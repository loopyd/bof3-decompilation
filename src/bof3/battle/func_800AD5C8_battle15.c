#include "bof3/battle/battle15_internal.h"

/* @source 0x800AD5C8
 * @behavior Scans the active byte list for the value of work byte 0x0B; on a
 *   match it publishes two halfword offsets derived from the D_80148628 pair
 *   into work halfwords 0x2E/0x30 and returns, otherwise it decrements work
 *   byte 2.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * The loop bound is re-derived from the count byte's address each iteration
 * (sibling shape of copyLocalTemplates): the original keeps the guard's count
 * address in v1 and a separate loop register holding the same address, which
 * only a second in-loop derivation reproduces.
 */
void func_800AD5C8(void)
{
  u8 *count;
  u8 *loop_count;
  u16 *pair;
  u8 *work;
  u32 target;
  u8 i;

  count = &D_801463C7;
  i = 0;
  if (*count != 0) {
    pair = (u16 *)&D_80148628;
    work = g_battle_work;
    target = work[0xB];
    do {
      if (D_801463C4[i] == target) {
        *(u16 *)(work + 0x2E) = pair[0] + (i * 24 + 0x41);
        *(u16 *)(work + 0x30) = pair[1] + 0x12;
        return;
      }
      loop_count = &D_801463C7;
      i++;
    } while (i < *loop_count);
  }
  g_battle_work[2]--;
}
