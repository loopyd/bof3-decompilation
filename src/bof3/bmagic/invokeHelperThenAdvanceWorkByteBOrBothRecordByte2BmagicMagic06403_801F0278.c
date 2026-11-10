#include "bof3/bof3.h"

extern u32 D_80143E6C; /* @source 0x80143E6C @kind data */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */
extern void func_801F03D8(void); /* @source 0x801F03D8 @kind function */

/* @source 0x801F0278
 * @behavior Calls the local helper at 0x801F03D8, then tests bit 0 of
 * 0x80143E6C. When clear, increments scratchpad work byte 0xB while it is
 * below 24; otherwise increments byte 2 of the shared record at 0x801EB4E0
 * and then byte 2 of the reloaded scratchpad work record. Published in the
 * payload callback table at offset 0x64; consumes no arguments.
 * @status exact
 * @match 100.00
 * @residual none
 */
void invokeHelperThenAdvanceWorkByteBOrBothRecordByte2BmagicMagic06403_801F0278(void) {
  u8 *work;
  u32 count;

  func_801F03D8();
  if ((D_80143E6C & 1) == 0) {
    work = g_battle_work;
    count = work[0xB];
    if (count >= 0x18) {
      D_801EB4E0[2]++;
      g_battle_work[2]++;
    } else {
      work[0xB] = count + 1;
    }
  }
}
