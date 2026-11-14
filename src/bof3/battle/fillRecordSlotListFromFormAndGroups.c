#include "bof3/battle/battle15_internal.h"

extern u8 D_800B4EA4[][3]; /* @source 0x800B4EA4 @kind table */
extern u8 D_800B6F58; /* @source 0x800B6F58 @kind data */
extern s8 D_800B6F60; /* @source 0x800B6F60 @kind data */
extern s8 D_800B6F61; /* @source 0x800B6F61 @kind data */
extern s8 D_800B6F62; /* @source 0x800B6F62 @kind data */
extern s8 D_800B6F63; /* @source 0x800B6F63 @kind data */
extern s8 D_800B6F64; /* @source 0x800B6F64 @kind data */
extern s8 D_800B6F69; /* @source 0x800B6F69 @kind data */
extern s8 D_800B6F6B; /* @source 0x800B6F6B @kind data */
extern s8 D_800B6F6C; /* @source 0x800B6F6C @kind data */

/* @source 0x800A78B8
 * @behavior Fills the ten-entry slot list at +0x70 of the active battler record
 * 0x80145F04 + D_80146374 * 0x140. The three bytes of the row selected by the
 * form byte 0x800B6F58 in the three-byte table 0x800B4EA4 are copied in order,
 * stopping at the first zero byte. Each of 0x800B6F60 through 0x800B6F64 that
 * is positive appends its group (0 through 4) through func_800A7B40, and each
 * of 0x800B6F61 through 0x800B6F64 that is positive appends the companion
 * group five above it (6 through 9) only while the shared gate byte 0x800B6F69
 * is positive; 0x800B6F60 also appends group 5 under that gate and group 0xE
 * under the positive 0x800B6F63, and the remaining groups 0xC, 0xB and 0xD are
 * appended under their own 0x800B6F6C, 0x800B6F6B and 0x800B6F69 bytes. The
 * byte 0xD9 is then written at the resulting count and every remaining entry up
 * to ten is zeroed.
 * @status exact
 * @match 100.00
 * @residual none
 */
void fillRecordSlotListFromFormAndGroups(void) {
  u8* record;
  u8 count;
  u8 i;
  u8 value;

  count = 0;
  i = 0;
  record = &battleWorkRecordBase[(u32)D_80146374 * 0x140u];

  for (; i < 3u; i++) {
    value = D_800B4EA4[D_800B6F58][i];
    if (value == 0u) {
      break;
    }
    (record + count)[0x70] = value;
    count++;
  }
  if (D_800B6F60 > 0) {
    count = func_800A7B40(0, count);
    if (D_800B6F69 > 0) {
      count = func_800A7B40(5, count);
    }
    if (D_800B6F63 > 0) {
      count = func_800A7B40(0xE, count);
    }
  }
  if (D_800B6F61 > 0) {
    count = func_800A7B40(1, count);
    if (D_800B6F69 > 0) {
      count = func_800A7B40(6, count);
    }
  }
  if (D_800B6F62 > 0) {
    count = func_800A7B40(2, count);
    if (D_800B6F69 > 0) {
      count = func_800A7B40(7, count);
    }
  }
  if (D_800B6F63 > 0) {
    count = func_800A7B40(3, count);
    if (D_800B6F69 > 0) {
      count = func_800A7B40(8, count);
    }
  }
  if (D_800B6F64 > 0) {
    count = func_800A7B40(4, count);
    if (D_800B6F69 > 0) {
      count = func_800A7B40(9, count);
    }
  }
  if (D_800B6F6C > 0) {
    count = func_800A7B40(0xC, count);
  }
  if (D_800B6F6B > 0) {
    count = func_800A7B40(0xB, count);
  }
  if (D_800B6F69 > 0) {
    count = func_800A7B40(0xD, count);
  }
  (record + count)[0x70] = 0xD9;
  for (count++; count < 10u; count++) {
    (record + count)[0x70] = 0;
  }
}
