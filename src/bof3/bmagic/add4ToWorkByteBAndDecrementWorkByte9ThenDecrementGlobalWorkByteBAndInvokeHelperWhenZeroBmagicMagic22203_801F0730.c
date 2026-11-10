#include "bof3/bof3.h"

void func_801E5988(void);

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* @source 0x801F0730
 * @behavior Work-byte-2 handler, entry 4 of the five-entry pointer table at
 *           0x801F0D98. Reads the scratchpad work pointer at 0x1F800044 and
 *           adds four to work byte 0xB. Re-reads that pointer and decrements
 *           unsigned work byte 9, testing the stored value truncated to eight
 *           bits. When it reaches zero, reads the shared record pointer at
 *           0x801EB4E0, decrements that record byte 0xB and calls the helper at
 *           0x801E5988. The record-byte store occupies the call delay slot.
 *           Takes no arguments and returns nothing. The 0x18-byte frame
 *           preserves only ra. Byte roles and the helper role remain unknown.
 *           The magic041 handler at 0x801EF358 supplies the statement shape:
 *           original payloads differ only in four byte-access offsets, using
 *           byte 0xB instead of 9 for the add and byte 9 instead of 0xA for
 *           the countdown. Those differences exclude byte-identical cloning.
 * @status exact
 * @match 100.00
 * @residual none
 */
void add4ToWorkByteBAndDecrementWorkByte9ThenDecrementGlobalWorkByteBAndInvokeHelperWhenZeroBmagicMagic22203_801F0730(void) {
  g_battle_work[0x0B] += 4;
  if (--g_battle_work[0x09] == 0) {
    D_801EB4E0[0x0B] -= 1;
    func_801E5988();
  }
}
