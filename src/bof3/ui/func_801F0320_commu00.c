#include "bof3/ui/commu00_internal.h"

/* @source 0x801F0320
 * @behavior Walks the sixty stride-9 local weight records at 0x801F2706 in
 *           lockstep with the stride-8 active-record table: when a record slot
 *           is active its kind byte selects one of the two container-local byte
 *           totals -- kind 10 adds the weight byte to the 0x801455C5 total and
 *           kind 11 adds it to the 0x801455C6 total.
 * @status partial
 * @match 93.18
 * @residual Pre-loop address-copy scheduling and the upper pointer's source register differ; both bodies are 176 bytes.
 * The integer address retains the original's separate symbol materialisation
 * and accumulator pointer. Deriving both pointers from one pointer folds away
 * that copy.
 */
void func_801F0320(void) {
  u8* total_a;
  u8* total_b;
  s32 totals_base;
  s32 woff;
  s32 offset;
  u8  kind;
  s32 match_a;
  s32 match_b;

  match_a = 10;
  match_b = 11;

  totals_base = (s32)&D_801455C5;
  total_b = (u8*)totals_base + 1;
  total_a = (u8*)totals_base;

  *total_a = 0;
  D_801455C6 = 0;

  offset = 0;
  for (woff = 0; woff < 540; woff += 9) {
    if (((volatile u8*)activeRecordBytes)[offset] != 0) {
      kind = D_801455C9[offset];

      if (kind == match_a) {
        *total_a = *total_a + D_801F2706[woff];
      } else if (kind == match_b) {
        *total_b = *total_b + D_801F2706[woff];
      }
    }

    offset += 8;
  }
}
