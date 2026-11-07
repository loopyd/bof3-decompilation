#include "bof3/bof3.h"

/* Shared scenario progress byte at 0x80146864, bound by the shared map. The
 * body reads it once through its symbol expression, which is the
 * `lui $v1,%hi` plus folded `lbu $v1,%lo` pair the original bytes show. */
extern u8 g_ScenarioProgress; /* @source 0x80146864 @kind unknown */

/* Shared mode byte at 0x80143BB0. The neighbouring handler 0x801F34D0 tests
 * the same byte against the same 3. */
extern u8 D_80143BB0; /* @source 0x80143BB0 @kind unknown */

/* Pointer cell of the shared record whose halfword at 0x0A this step consumes. */
extern u16* D_80148210; /* @source 0x80148210 @kind unknown */

/* @source 0x801F3458
 * @behavior Scratch work-record handler that runs only while the shared
 *           scenario progress byte at 0x80146864 is not 1. When the shared
 *           mode byte at 0x80143BB0 reads 3 it arms the work record published
 *           through the pointer cell at 0x1F800044: the record byte 0 flag bit
 *           0x40 is loaded into a value register, or-ed with 0x40 and stored
 *           back through the same record pointer, and a second load of the
 *           same cell then writes 1 into record byte 4. That second load is a
 *           fresh read of the cell, not the first pointer reused. Whatever the
 *           mode byte reads, the step then consumes two units of the halfword
 *           at 0x0A of the record published through the global pointer cell
 *           0x80148210. Both stores and the halfword update happen inside the
 *           progress gate, so progress 1 returns without touching either
 *           record. Takes no arguments, calls nothing and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setWorkFlag40AndByte4To1WhenMode3ThenDecrementHalfword0aUnlessScenarioProgress1World03Area14213_801F3458(
    void) {
  u8** slots;
  u8* work;
  u8 value;

  if (g_ScenarioProgress != 1) {
    if (D_80143BB0 == 3) {
      slots = SPAD_PTR_TABLE(u8);
      work = slots[0x11];
      value = work[0];
      value |= 0x40;
      work[0] = value;
      slots = SPAD_PTR_TABLE(u8);
      slots[0x11][4] = 1;
    }
    D_80148210[5] -= 2;
  }
}
