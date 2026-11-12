#include "bof3/bof3.h"

/* Scratchpad work-record pointer cell. Naming the cell keeps the original's
 * per-access re-materialization (lui $vN,0x1F80 / lw $vN,68($vN)); the bare
 * constant address makes the compiler CSE 0x1F800044 into one saved register
 * and load through offset zero instead.
 * @source 0x1F800044 @kind data
 */
extern u8* D_1F800044;

/* Block drawer at 0x801F4E70, still unlifted assembly inside this target, so
 * only its prototype is declared. It draws the four-way block set that the
 * ramp and step handlers of this overlay select with a variant index. */
void func_801F4E70(u32 variant);

/* Shared request helper of the main executable, outside this overlay's payload
 * and reached only through this body and its sibling handlers. Its role is
 * unresolved: the exact lifts that call it treat it as a two-value request
 * helper, so the parameters stay structural. This target's generated
 * undefined-funcs map binds the address, so no map row is added. */
extern void func_8015507C(s32 arg0, s32 arg1); /* @source 0x8015507C */

/* @source 0x801F465C
 * @behavior Overlay step handler, entry 14 of the work-byte-2 handler table at
 *           0x801F5954 and entry 17 of the work-byte-1 handler table at
 *           0x801F5948: reads the work record published at the scratchpad
 *           pointer cell 0x1F800044 and rewinds its byte 0x09 to zero when that
 *           byte stands at 0x0F. A second cell read reads the signed halfword
 *           at 0x32 as an arm countdown: while it stays positive the handler
 *           decrements it and leaves the dispatch step alone, and otherwise it
 *           advances the record's byte 0x02 through that same pointer only
 *           while the band index (byte 0x09 / 5 truncated to a byte) is still
 *           zero. It then hands the block drawer func_801F4E70 the band index
 *           (byte 0x09 / 5) + 7 on every tick whose byte 0x09 is a multiple of
 *           five, advances byte 0x09 by one through a fresh cell read, and
 *           finally re-reads the cell to pass the shared request helper
 *           func_8015507C the selector 0x42 and the level (halfword 0x32 / 6) +
 *           0x18 truncated to 16 bits. Takes no arguments, returns nothing and
 *           uses a 0x20-byte frame that keeps the return address at 0x18. The
 *           role of the halfword at 0x32 beyond that countdown, and of the
 *           shared request helper's parameters, stay described by address.
 * @status exact
 * @match 100.00
 * @residual none
 */
void countDownHalfword32ElseAdvanceWorkByte2ThenDrawOnFifthTickAndRequestPair42World02Area10313_801F465C(
    void) {
  u8* work;
  u8* armCursor;
  u8* tickCursor;
  u8* stepCursor;
  u8* levelCursor;
  u32 ticks;
  s16 level;

  work = D_1F800044;
  if (work[9] == 0x0F) {
    work[9] = 0;
  }
  armCursor = D_1F800044;
  if (*(s16*)(armCursor + 0x32) > 0) {
    *(s16*)(armCursor + 0x32) -= 1;
  } else if ((u8)(armCursor[9] / 5) == 0) {
    armCursor[2]++;
  }
  tickCursor = D_1F800044;
  ticks = tickCursor[9];
  if ((u8)(ticks % 5) == 0) {
    func_801F4E70((u8)(ticks / 5) + 7);
  }
  stepCursor = D_1F800044;
  stepCursor[9]++;
  levelCursor = D_1F800044;
  level = (s16) * (u16*)(levelCursor + 0x32) / 6 + 0x18;
  func_8015507C(0x42, level);
}
