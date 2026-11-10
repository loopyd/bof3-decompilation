#include "bof3/bof3.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */
extern u8 D_80146384; /* @source 0x80146384 @kind unknown */
extern u8 D_801EB630[]; /* @source 0x801EB630 @kind table */
extern u8 D_80145E90[]; /* @source 0x80145E90 @kind table */

extern void func_800A9820(u8 index); /* @source 0x800A9820 @kind function */
extern void func_80196718(void *record); /* @source 0x80196718 @kind function */
extern void func_800AF814(u8 index); /* @source 0x800AF814 @kind function */

/* @source 0x801EF264
 * @behavior When byte 0xB of the record pointed to by 0x801EB4E0 is below
 *   eight, forward work byte 3 to 0x800A9820, submit its selected record to
 *   0x80196718, then forward the index to 0x800AF814 and increment work byte 2.
 *   Bit 0x40 of 0x80146384 selects the 0x118-byte records at 0x801EB630 and
 *   adds three to each forwarded byte index; otherwise select the 0x140-byte
 *   records at 0x80145E90 without that adjustment. Re-read the scratchpad work
 *   pointer after each call. Helper results are unused; there are no arguments
 *   and no return value. The entry is stored in the local handler table.
 * @status exact
 * @match 100.00
 * @residual none
 */
void forwardWorkByte3SubmitSelectedRecordAndAdvanceWorkByte2WhenGlobalByteBBelow8BmagicMagic09303_801EF264(void) {
  if (D_801EB4E0[0xB] < 8u) {
    if (D_80146384 & 0x40) {
      func_800A9820((u8)(g_battle_work[3] + 3));
      func_80196718(D_801EB630 + g_battle_work[3] * 0x118u);
      func_800AF814((u8)(g_battle_work[3] + 3));
    } else {
      func_800A9820(g_battle_work[3]);
      func_80196718(D_80145E90 + g_battle_work[3] * 0x140u);
      func_800AF814(g_battle_work[3]);
    }
    g_battle_work[2]++;
  }
}
