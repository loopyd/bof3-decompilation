#include "bof3/ui/commu00_internal.h"

/* @source 0x801F0320
 * @behavior Walks the sixty stride-9 local weight records at 0x801F2706 in
 *           lockstep with the stride-8 active-record table: when a record slot
 *           is active its kind byte selects one of the two container-local byte
 *           totals -- kind 10 adds the weight byte to the 0x801455C5 total and
 *           kind 11 adds it to the 0x801455C6 total.
 * @status partial
 * @match 97.73
 * @residual 43/44 instructions match at 176 bytes. The active-record offset
 * is zeroed before the weight-table address rather than after it.
 */
void func_801F0320(void) {
  u8* total_a;
  u8* total_b;
  s32 address_or_weight_offset;
  s32 offset;
  u8  kind;
  s32 match_a;
  s32 match_b;

  match_a = 10;
  match_b = 11;

  address_or_weight_offset = (s32)&D_801455C5;
  total_a = (u8*)address_or_weight_offset;
  total_b = total_a + 1;

  *total_a = 0;
  D_801455C6 = 0;

  offset = 0;
  for (address_or_weight_offset = 0; address_or_weight_offset < 540;
       address_or_weight_offset += 9) {
    if (((volatile u8*)activeRecordBytes)[offset] != 0) {
      kind = D_801455C9[offset];

      if (kind == match_a) {
        *total_a = *total_a + D_801F2706[address_or_weight_offset];
      } else if (kind == match_b) {
        *total_b = *total_b + D_801F2706[address_or_weight_offset];
      }
    }

    offset += 8;
  }
}
