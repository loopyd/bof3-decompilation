#include "bof3/bof3.h"

/* Work-table tick helper, the overlay's own routine at 0x801F3AAC. Its body
 * walks the 0x80 entries of the 0x14-byte-stride work table at 0x800E4800 that
 * the exact sibling clearWorkTableAt800E4800World02Area08213_801F3910 names,
 * reloads the scratch work record through 0x1F800044 inside the loop and
 * reports through a zero-extended byte whether any entry still carried a
 * non-zero lead byte this frame. Splat binds the address through this target's
 * generated undefined_funcs_auto.txt, so no target-map row is needed. */
u8 func_801F3AAC(void);

/* @source 0x801F3470
 * @behavior Overlay step handler, entry index 10 of the twelve-pointer step
 *           table at 0x801F5734 (0x801F2C6C, 0x801F2C9C, 0x801F2CF0,
 *           0x801F2D30, 0x801F2D90, 0x801F2DE4, 0x801F2EC8, 0x801F336C,
 *           0x801F3398, 0x801F3408, 0x801F3470, 0x801F34B8; the table ends at
 *           0x801F5764, where two 0x801F-flow words lead into the byte-2
 *           handler table at 0x801F576C). Per frame it calls the work-table
 *           tick helper func_801F3AAC and tests the returned byte: while the
 *           helper reports at least one active work-table entry the handler
 *           does nothing, and once it reports none it re-reads the scratchpad
 *           work pointer cell 0x1F800044, writes the halfword 0x10 to the work
 *           record's +0x32 field and increments the record's +0x01 step byte
 *           with a byte-width add (lbu / addiu 1 / sb). The function has a
 *           0x18-byte frame holding the saved return address at 0x10, reads or
 *           writes no other state, and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setWorkHalfword32To16WhenWorkTableIdleIncrementWorkByte1World02Area08213_801F3470(
    void) {
  u8* work;

  if (func_801F3AAC() == 0u) {
    work = SPAD_PTR_SLOT(u8, 0x44);
    *(u16*)(work + 0x32) = 0x10;
    work[1]++;
  }
}
