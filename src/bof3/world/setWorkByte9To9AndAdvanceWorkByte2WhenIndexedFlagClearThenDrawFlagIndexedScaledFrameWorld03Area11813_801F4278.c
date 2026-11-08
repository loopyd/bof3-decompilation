#include "bof3/bof3.h"

extern u8 D_80144ED0; /* @source 0x80144ED0 @kind unknown */
extern u8 D_801F5138[];
extern u8 D_801F5198[];
extern u8* D_1F800044; /* @source 0x1F800044 @kind bss */

s32 func_8015B5D4(u32 arg0, s32 arg1);
void drawRecordCellTexturedQuadWorld03Area11813_801F4634(s32 arg0, s32 arg1);
void func_801F48E0(void);

/* @source 0x801F4278
 * @behavior Byte-2 state handler that the last byte-2 dispatch table of this
 *           overlay (0x801F519C, indexed by the dispatcher at 0x801F410C)
 *           lists at entry 1, and that the handler at 0x801F4150 calls
 *           directly on the branch where its own probe of the flag bank at
 *           0x80144ED0 with the flag number from table 0x801F5198 reports set.
 *           Reads the work record published at the scratchpad pointer cell
 *           0x1F800044, takes its cursor byte 0x0B as an index into the
 *           four-byte flag-number table 0x801F5198 and probes that flag number
 *           in the flag bank through the shared flag tester func_8015B5D4.
 *           When that flag is clear, stores 9 into the record byte at 0x09,
 *           the countdown its neighbour at 0x801F4340 decrements before
 *           advancing the same record state byte, and increments the record
 *           state byte at 0x02 so the next dispatch takes entry 2 of the same
 *           table (0x801F4340). It then takes the same cursor byte 0x0B as an
 *           index into the four-byte flag-number table 0x801F5138, probes that
 *           flag number the same way and draws the record through the overlay
 *           record draw routine 0x801F4634 with frame index record byte 0x0B
 *           plus four times that flag result plus one, so the frame steps by
 *           four once the indexed flag is set, at size 0x40. Every path
 *           finishes with the overlay primitive setup 0x801F48E0. Takes no
 *           arguments and returns nothing; the record pointer is loaded before
 *           the frame is created, the 0x18-byte frame keeps only $ra at 0x14
 *           and $s0 at 0x10, which holds the flag bank base across both
 *           probes, and the record cell is reloaded for each of the five uses.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setWorkByte9To9AndAdvanceWorkByte2WhenIndexedFlagClearThenDrawFlagIndexedScaledFrameWorld03Area11813_801F4278(
    void) {
  if (func_8015B5D4((u32)&D_80144ED0, D_801F5198[D_1F800044[0xB]]) == 0) {
    D_1F800044[9] = 9;
    D_1F800044[2]++;
  }
  drawRecordCellTexturedQuadWorld03Area11813_801F4634(
      D_1F800044[0xB] +
          (func_8015B5D4((u32)&D_80144ED0, D_801F5138[D_1F800044[0xB]]) << 2) +
          1,
      0x40);
  func_801F48E0();
}
