#include "bof3/bof3.h"

extern u8 *D_8014637C; /* @source 0x8014637C @kind data */
extern u8 D_80146384; /* @source 0x80146384 @kind data */
extern u8 D_801EB659[]; /* @source 0x801EB659 @kind data */
extern u8 D_80145EB9[]; /* @source 0x80145EB9 @kind data */
extern u8 D_801462EA; /* @source 0x801462EA @kind data */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared main-executable availability test: nonzero means inactive or flagged
 * 0x4000. The caller passes an unsigned byte and consumes an unsigned byte. */
extern u8 func_801DB524(u8 selector); /* @source 0x801DB524 */

/* @source 0x801EF404
 * @behavior Sets byte 0x29 of the record at 0x8014637C to 2. If bit 0x40
 *           of 0x80146384 is set, scans eight enemy selectors 3..10 and sets
 *           byte 0x29 of each available stride-0x118 record to 2; otherwise
 *           scans three party selectors 0..2 and sets the corresponding byte
 *           of each available stride-0x140 record to 2. Sets work byte 0x29
 *           to 7 when 0x801462EA is 0x37, or 3 otherwise, then sets work byte
 *           0xB to 1, clears bytes 9 and 0xA, and advances work byte 2.
 *           The work pointer cell is re-read between the byte stores. This
 *           no-argument handler is word 4 of the work-byte-1 dispatch table
 *           at 0x801F0388. Unknown byte roles remain offset-anchored.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setEligibleBattlerByte29ToTwoAndInitializeWorkBmagicMagic22503_801EF404(void) {
  u8 index;
  s32 offset;

  D_8014637C[0x29] = 2;
  if ((D_80146384 & 0x40) != 0) {
    index = 0;
    do {
      if (func_801DB524((u8)(index + 3)) == 0) {
        offset = index * 0x118;
        D_801EB659[offset] = 2;
      }
      index++;
    } while (index < 8);
  } else {
    index = 0;
    do {
      if (func_801DB524(index) == 0) {
        offset = index * 0x140;
        D_80145EB9[offset] = 2;
      }
      index++;
    } while (index < 3);
  }
  if (D_801462EA == 0x37) {
    g_battle_work[0x29] = 7;
  } else {
    g_battle_work[0x29] = 3;
  }
  g_battle_work[0xB] = 1;
  g_battle_work[9] = 0;
  g_battle_work[0xA] = 0;
  g_battle_work[2]++;
}
