#include "bof3/bof3.h"

/* @source 0x1F800044 @kind unknown */
extern u8* workRecordCursor1F800044;

/* @source 0x801F3914 @kind function */
extern void func_801F3914(void);

/* @source 0x801F3738
 * @behavior Reads the scratchpad work pointer at 0x1F800044, adds 0x10 to
 *           halfword +0x30, and advances dispatch byte +2 when the updated
 *           signed halfword is at least 0x80. Then calls local helper
 *           0x801F3914. Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateAdvanceWorkCall3914(void) {
  u8* work;
  s32 offset;

  work = workRecordCursor1F800044;
  offset = FIELD_REF(u16, work, 0x30) + 0x10;
  FIELD_REF(u16, work, 0x30) = offset;
  if ((s16)offset >= 0x80) {
    work[2]++;
  }
  func_801F3914();
}
