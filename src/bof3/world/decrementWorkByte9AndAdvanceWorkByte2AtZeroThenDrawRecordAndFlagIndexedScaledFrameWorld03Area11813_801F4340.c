#include "bof3/bof3.h"

extern u8 D_80144ED0; /* @source 0x80144ED0 @kind unknown */
extern u8 D_801F5138[];
extern u8* D_1F800044; /* @source 0x1F800044 @kind bss */

s32 func_8015B5D4(u32 arg0, s32 arg1);
void func_801F4634(s32 arg0, s32 arg1);
void func_801F48E0(void);

/* @source 0x801F4340
 * @behavior Byte-2 state handler that the last byte-2 dispatch table of this
 *           overlay (0x801F519C, indexed by the dispatcher at 0x801F410C)
 *           lists at entry 2. Decrements the work record byte at offset 0x09
 *           and, once that countdown reads zero, increments the record state
 *           byte at offset 0x02 so the next dispatch takes entry 3 of the same
 *           table (0x801F43FC). Draws the record through the overlay record
 *           draw routine 0x801F4634 with mode 0 and size 0x40, then takes the
 *           record cursor byte at offset 0x0B as an index into the four-byte
 *           flag-number table 0x801F5138, probes that flag number in the flag
 *           bank at 0x80144ED0 through the shared flag tester func_8015B5D4
 *           and draws the record again with frame index record byte 0x0B plus
 *           four times that flag result plus one, so the frame steps by four
 *           once the indexed flag is set, at size record byte 0x09 times eight,
 *           so the drawn size shrinks with the countdown. Every path finishes
 *           with the overlay primitive setup 0x801F48E0. Takes no arguments and
 *           returns nothing; the record pointer is loaded before the frame is
 *           created, the 0x18-byte frame only keeps $ra at 0x10, and the
 *           record cell is reloaded for each use.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementWorkByte9AndAdvanceWorkByte2AtZeroThenDrawRecordAndFlagIndexedScaledFrameWorld03Area11813_801F4340(
    void) {
  D_1F800044[9]--;
  if (D_1F800044[9] == 0) {
    D_1F800044[2]++;
  }
  func_801F4634(0, 0x40);
  func_801F4634(
      D_1F800044[0xB] +
          (func_8015B5D4((u32)&D_80144ED0, D_801F5138[D_1F800044[0xB]]) << 2) +
          1,
      D_1F800044[9] << 3);
  func_801F48E0();
}
