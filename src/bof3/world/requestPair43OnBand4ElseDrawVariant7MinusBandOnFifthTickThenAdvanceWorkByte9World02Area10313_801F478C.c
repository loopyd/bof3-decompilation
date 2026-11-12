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
 * and reached from this body only for the terminal band. Its role is
 * unresolved: the exact lifts that call it treat it as a two-value request
 * helper, so the parameters stay structural. This target's generated
 * undefined-funcs map binds the address, so no map row is added.
 * @source 0x80155218
 */
extern void func_80155218(s32 arg0, s32 arg1);

/* Second shared helper of the main executable, also outside this overlay's
 * payload. This body calls it once, right after the request above, and its role
 * is unresolved, so it keeps only this prototype. This target's generated
 * undefined-funcs map binds the address, so no map row is added.
 * @source 0x80196070
 */
extern void func_80196070(void);

/* @source 0x801F478C
 * @behavior Overlay step handler, entry 15 of the work-byte-2 handler table at
 *           0x801F5954 and entry 18 of the work-byte-1 handler table at
 *           0x801F5948: reads the work record published at the scratchpad
 *           pointer cell 0x1F800044 and forms the band index byte 0x09 / 5.
 *           Once that index stands at 4, the terminal band, it hands the shared
 *           request helper func_80155218 the pair (0x43, 0x1A), calls the
 *           shared helper func_80196070, and returns without advancing byte
 *           0x09. Every lower band instead hands the block drawer func_801F4E70
 *           the variant 7 minus the band index on each tick whose byte 0x09 is
 *           a multiple of five, then advances byte 0x09 by one through a fresh
 *           cell read. Takes no arguments, returns nothing and uses a 0x18-byte
 *           frame that holds only the saved return address at 0x10. The roles
 *           of the two shared helpers beyond those uses stay described by
 *           address.
 * @status exact
 * @match 100.00
 * @residual none
 */
void requestPair43OnBand4ElseDrawVariant7MinusBandOnFifthTickThenAdvanceWorkByte9World02Area10313_801F478C(
    void) {
  u8* work;
  u8* stepCursor;
  u32 ticks;
  u32 band;

  work = D_1F800044;
  ticks = work[9];
  band = ticks / 5;
  if ((u8)band == 4) {
    func_80155218(0x43, 0x1A);
    func_80196070();
  } else {
    if ((u8)(ticks % 5) == 0) {
      func_801F4E70(7 - (u8)band);
    }
    stepCursor = D_1F800044;
    stepCursor[9]++;
  }
}
