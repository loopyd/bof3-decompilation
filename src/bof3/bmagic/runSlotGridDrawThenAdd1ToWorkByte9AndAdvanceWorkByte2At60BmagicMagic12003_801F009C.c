#include "bof3/bof3.h"

/* Local three-by-eight slot-grid draw routine. Its entry reads no arguments;
 * this caller ignores its result. */
extern void func_801F0274(void); /* @source 0x801F0274 */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801F009C
 * @behavior Runs the target-local slot-grid draw routine at 0x801F0274, then
 *           increments work-object byte 9 modulo 256 through the scratchpad
 *           pointer cell at 0x1F800044. When the stored byte equals 0x60,
 *           reloads that cell and increments work-object byte 2 modulo 256.
 *           Takes no arguments and returns nothing. The slot-grid routine
 *           visits three rows of eight slots, publishes work bytes 0xB and
 *           0xA, and draws accepted slots. This entry is stored at 0x801F0CD0
 *           in the overlay's handler table. Byte roles remain offset-based.
 * @status exact
 * @match 100.00
 * @residual none
 */
void runSlotGridDrawThenAdd1ToWorkByte9AndAdvanceWorkByte2At60BmagicMagic12003_801F009C(
    void) {
  func_801F0274();
  if ((g_battle_work[9] += 1) == 0x60) {
    g_battle_work[2]++;
  }
}
