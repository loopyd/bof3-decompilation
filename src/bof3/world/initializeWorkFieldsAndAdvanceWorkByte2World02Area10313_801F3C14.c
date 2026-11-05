#include "bof3/bof3.h"

/* Scratchpad work-record pointer cell. Naming the cell keeps the original's
 * per-access re-materialization (lui $vN,0x1F80 / lw $vN,68($vN)); a bare
 * constant address makes the compiler CSE 0x1F800044 into one register and
 * load through offset zero instead.
 * @source 0x1F800044 @kind data
 */
extern u8* D_1F800044;

/* @source 0x801F3C14
 * @behavior Overlay step handler, entry 0 of the work-byte-2 handler table at
 *           0x801F5954: reads the work record published at scratchpad pointer
 *           cell 0x1F800044 and clears its byte at 0x09, re-reads the cell and
 *           seeds the record's fixed-point position pair with x = 0x570000 at
 *           word 0x34 and y = 0x140000 at word 0x38, stores 0xFF00 (signed
 *           -0x100) in the halfword at 0x3E and 0 in the halfword at 0x32,
 *           clears the byte at 0x0B through that second cell read, then
 *           re-reads the cell once more and advances the record's byte-2
 *           dispatch step by one. Takes no arguments and returns nothing; the
 *           roles of the bytes at 0x09/0x0B and the halfwords at 0x32/0x3E are
 *           unknown.
 * @status exact
 * @match 100.00
 * @residual none
 * Each of the three cell reads is its own local so the pointer cell is
 * re-materialized at the original reload points; merging them into one local
 * would emit a single load.
 */
void initializeWorkFieldsAndAdvanceWorkByte2World02Area10313_801F3C14(void) {
  u8* work;
  u8* cursor;
  u8* step;

  work = D_1F800044;
  work[9] = 0;
  cursor = D_1F800044;
  *(s32*)(work + 0x34) = 0x570000;
  *(s32*)(work + 0x38) = 0x140000;
  *(s16*)(work + 0x3E) = -0x100;
  *(s16*)(work + 0x32) = 0;
  cursor[0xB] = 0;
  step = D_1F800044;
  step[2]++;
}
