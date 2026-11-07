#include "bof3/bof3.h"

extern s32 D_80143E6C;
extern u8 g_ScenarioProgress;

/* Work-table entry spawner, the overlay's own routine at 0x801F393C. Its body
 * walks the 0x80 entries of the 0x14-byte-stride work table at 0x800E4800,
 * claims the first entry whose lead byte is still zero by storing 1 there and
 * 0x40 at +0x01, fills that entry's +0x02/+0x06/+0x0A words from rand() scaled
 * by the record words +0x34/+0x38 and the halfwords +0x2E/+0x30, seeds the
 * entry halfword from rand() plus 0x1000 and returns nothing. Splat binds the
 * address through this target's own asm subsegments, so no target-map row is
 * needed. */
void func_801F393C(void);

/* Work-table tick helper, the overlay's own routine at 0x801F3AAC. Its body
 * walks the same 0x80 entries of the 0x14-byte-stride work table, reloads the
 * scratch work record through 0x1F800044 inside the loop and reports through a
 * zero-extended byte whether any entry still carried a non-zero lead byte this
 * frame. Splat binds the address through this target's own asm subsegments, so
 * no target-map row is needed. */
void func_801F3AAC(void);

/* @source 0x801F3408
 * @behavior Overlay step handler, entry index 9 of the twelve-pointer step
 *           table at 0x801F5734 (0x801F2C6C, 0x801F2C9C, 0x801F2CF0,
 *           0x801F2D30, 0x801F2D90, 0x801F2DE4, 0x801F2EC8, 0x801F336C,
 *           0x801F3398, 0x801F3408, 0x801F3470, 0x801F34B8; the table ends at
 *           0x801F5764). On the odd frames of the shared per-frame counter
 *           0x80143E6C it spawns one work-table entry through the overlay's
 *           own routine at 0x801F393C, then ticks the work table through
 *           0x801F3AAC, and, only when the shared scenario progress byte at
 *           0x80146864 is 0x29, re-reads the scratchpad work pointer cell
 *           0x1F800044 and advances that work record's +0x01 step byte by one
 *           so the per-frame dispatcher moves on. The returned byte of the tick
 *           helper is discarded. The function has a 0x18-byte frame holding the
 *           saved return address at 0x10, takes no arguments and returns
 *           nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void spawnWorkEntryOnOddFrameTickWorkTableIncrementWorkByte1AtScenarioProgress29World02Area08213_801F3408(
    void) {
  u8* work;

  if (D_80143E6C & 1) {
    func_801F393C();
  }
  func_801F3AAC();
  if (g_ScenarioProgress == 0x29) {
    work = SPAD_PTR_SLOT(u8, 0x44);
    work[1]++;
  }
}
