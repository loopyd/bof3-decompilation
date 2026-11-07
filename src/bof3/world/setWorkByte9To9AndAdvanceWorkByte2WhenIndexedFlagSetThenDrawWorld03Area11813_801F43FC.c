#include "bof3/bof3.h"

extern u8 D_80144ED0; /* @source 0x80144ED0 @kind unknown */
extern u8 D_801F5198[];
extern u8* D_1F800044; /* @source 0x1F800044 @kind bss */

s32 func_8015B5D4(u32 arg0, s32 arg1);
void func_801F4634(s32 arg0, s32 arg1);

/* @source 0x801F43FC
 * @behavior Byte-2 state handler that the last byte-2 dispatch table of this
 *           overlay (0x801F519C, indexed by the dispatcher at 0x801F410C)
 *           lists at entry 3, and that the handler at 0x801F4150 calls
 *           directly on the branch where its own probe of flag bank 0x80144ED0
 *           reports clear. Reads the work record published at the scratchpad
 *           pointer cell 0x1F800044, takes its cursor byte 0x0B as an index
 *           into the four-byte flag-number table 0x801F5198 and probes that
 *           flag number in the flag bank at 0x80144ED0 through the shared flag
 *           tester func_8015B5D4. When the flag is set, stores 9 into the
 *           record byte at 0x09, which its neighbours at 0x801F4340 and
 *           0x801F447C count down before advancing the same record state byte,
 *           and increments the record state byte at 0x02 so the next dispatch
 *           takes the following entry of the same table. Every path finishes
 *           with the overlay record draw routine 0x801F4634 called with mode 0
 *           and size 0x40. Takes no arguments and returns nothing; the record
 *           pointer is loaded before the frame is created, the 0x18-byte frame
 *           only keeps $ra at 0x10 across the draw call, and the record cell is
 *           reloaded for each of the three uses.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setWorkByte9To9AndAdvanceWorkByte2WhenIndexedFlagSetThenDrawWorld03Area11813_801F43FC(
    void) {
  if (func_8015B5D4((u32)&D_80144ED0, D_801F5198[D_1F800044[0xB]]) != 0) {
    D_1F800044[9] = 9;
    D_1F800044[2]++;
  }
  func_801F4634(0, 0x40);
}
