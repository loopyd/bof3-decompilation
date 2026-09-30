#include "bof3/ui/commu00_internal.h"

/* @source 0x801EF214
 * @behavior Advances the village-expansion tier byte at 0x801455C4 once the
 *           shared battle counter has moved far enough past its snapshot:
 *           returns while the tier byte already reads seven, then, for each of
 *           the eight stride-8 local slots at 0x801457A8/0x801457A9 that carry
 *           slot kind four and a nonzero second byte, walks the sixty stride-8
 *           active-record entries and sums the container-local stride-9 byte
 *           column at 0x801F2708 over the present entries whose kind byte
 *           equals the slot index plus one, counting them; it returns when no
 *           entry matched. It then reads the counter word at 0x8014502C and its
 *           snapshot at 0x801455B8, requires the tier total at 0x801F2514 for
 *           the current tier (floored at one) to have elapsed, and advances the
 *           tier byte, adds that tier total to the snapshot word and appends
 *           notification kind 7 to the pending queue at 0x80145E60/0x80145E5D.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801EF214(void) {
  u32 total;
  u32 count;
  s32 i;
  s32 j;
  s32 k;
  s32 w;
  u32 level;
  u32 prev;
  u32 now;
  u8* state;
  u32 kind;
  u32 slot_kind;
  s16 need;

  total = 0;
  count = 0;
  if (D_801455C4 >= 7) {
    return;
  }

  i = 0;
  slot_kind = 4;
  j = 0;
  for (; i < 8; i++, j += 8) {
    if (D_801457A8[j] == slot_kind && D_801457A9[j] != 0) {
      kind = i + 1;
      w = 0;
      for (k = 0; k < 480; k += 8) {
        if (((volatile u8*)activeRecordBytes)[k] != 0 && D_801455C9[k] == kind) {
          total += D_801F2708[w];
          count++;
        }
        w += 9;
      }
    }
  }

  if ((u8)count == 0) {
    return;
  }

  state = &D_801455C4;
  level = *state;
  need = (s16)(D_801F2514[level] - total);
  if (need <= 0) {
    need = 1;
  }

  now = D_8014502C;
  prev = D_801455B8;
  if (now - prev < (u32)need) {
    return;
  }

  *state = level + 1;
  D_801455B8 = prev + need;
  ((u8*)D_80145E60)[D_80145E5D * 2] = 7;
  D_80145E5D = D_80145E5D + 1;
}
