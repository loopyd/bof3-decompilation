#include "bof3/bof3.h"

/* @source 0x1F800044 @kind unknown */
extern u8* D_1F800044;

/* Two-byte row of the work step row table at 0x801F35C0. The original scales
 * the signed step selector in work halfword +0x36 by two and reads both bytes
 * of one row; each row byte becomes one work halfword. */
typedef struct World04Area16513WorkStepRow35c0 {
  u8 field_00;
  u8 field_01;
} World04Area16513WorkStepRow35c0;

/* @source 0x801F35C0 @kind table */
extern World04Area16513WorkStepRow35c0 workStepRowTable35c0World04Area16513[];

/* @source 0x801F345C
 * @behavior State handler for the work record published at the scratchpad
 *           pointer cell 0x1F800044. Copies the row of the two-byte work step
 *           row table at 0x801F35C0 selected by the record's signed halfword
 *           +0x36 back into that halfword and into halfword +0x3A, sets
 *           halfword +0x3E to -0x80 and halfword +0x30 to 0x100, and advances
 *           the record's state byte +0x02 by one. Before the row is copied,
 *           the record's byte +0x08 takes one when the halfword +0x3A read
 *           ahead of the cell reload is zero. The cell is loaded once for that
 *           leading flag group and reloaded for the row group. Takes no
 *           arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void loadWorkStepRowResetWorkFieldsAndAdvanceState2World04Area16513_801F345C(
    void) {
  u8* work;
  u8* next;
  s32 index;

  work = D_1F800044;
  index = *(s16*)(work + 0x36);
  *(u8*)(work + 8) = (*(s16*)(work + 0x3A) == 0);
  next = D_1F800044;
  *(u16*)(next + 0x36) = workStepRowTable35c0World04Area16513[index].field_00;
  *(u16*)(next + 0x3A) = workStepRowTable35c0World04Area16513[index].field_01;
  *(s16*)(next + 0x3E) = -0x80;
  *(u16*)(next + 0x30) = 0x100;
  next[2]++;
}
