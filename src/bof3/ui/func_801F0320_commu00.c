#include "bof3/ui/commu00_internal.h"

/* @source 0x801F0320
 * @behavior Walks the sixty stride-9 local weight records at 0x801F2706 in
 *           lockstep with the stride-8 active-record table: when a record slot
 *           is active its kind byte selects one of the two container-local byte
 *           totals -- kind 10 adds the weight byte to the 0x801455C5 total and
 *           kind 11 adds it to the 0x801455C6 total.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801F0320(void) {
  u8* total_a;
  u8* total_b;
  s32 address_or_kind;
  s32 offset;
  u8* weight;
  u8* end;
  s32 match_a;
  s32 match_b;

  match_a = 10;
  match_b = 11;

  address_or_kind = (s32)&D_801455C5;
  total_a = (u8*)address_or_kind;
  total_b = total_a + 1;

  *total_a = 0;
  D_801455C6 = 0;

  weight = D_801F2706;
  offset = 0;
  end = weight + 540;
  do {
    if (((volatile u8*)activeRecordBytes)[offset] != 0) {
      address_or_kind = D_801455C9[offset];

      if (address_or_kind == match_a) {
        *total_a = *total_a + *weight;
      } else if (address_or_kind == match_b) {
        *total_b = *total_b + *weight;
      }
    }

    offset += 8;
    weight += 9;
  } while ((s32)weight < (s32)end);
}
