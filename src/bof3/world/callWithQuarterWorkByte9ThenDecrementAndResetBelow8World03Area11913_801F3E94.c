#include "bof3/bof3.h"

extern u8* world03Area11913WorkRecord; /* @source 0x1F800044 @kind data */
extern void func_801F3F04(s32 arg0); /* @source 0x801F3F04 */
extern void func_80196070(void); /* @source 0x80196070 */

/* @source 0x801F3E94
 * @behavior Fourth handler in the table at 0x801F4634, indexed by work byte 2.
 * Calls 0x801F3F04 with the unsigned work byte 9 shifted right by two, then
 * decrements that byte at byte width. Re-reads the work-record pointer after
 * the call and the byte store. If the stored byte is below 8, calls the shared
 * work-area reset helper at 0x80196070. Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void callWithQuarterWorkByte9ThenDecrementAndResetBelow8World03Area11913_801F3E94(void) {
  u8* work;
  u8* step;

  func_801F3F04(world03Area11913WorkRecord[9] >> 2);
  work = world03Area11913WorkRecord;
  work[9] -= 1;
  step = world03Area11913WorkRecord;
  if (step[9] < 8) {
    func_80196070();
  }
}
