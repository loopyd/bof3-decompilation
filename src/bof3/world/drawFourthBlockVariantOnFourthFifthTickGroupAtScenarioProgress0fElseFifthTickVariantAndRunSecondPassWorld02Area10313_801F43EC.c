#include "bof3/bof3.h"

/* Scratchpad work-record pointer cell. Naming the cell keeps the original's
 * per-access re-materialization (lui $vN,0x1F80 / lw $vN,68($vN)); the bare
 * constant address makes the compiler CSE 0x1F800044 into one saved register
 * and load through offset zero instead.
 * @source 0x1F800044 @kind data
 */
extern u8* D_1F800044;

/* Shared scenario progress byte of the main executable, outside this overlay's
 * payload. This handler draws its fourth block variant only while the byte
 * reads 0x0F and otherwise takes the fifth-tick variant path; the progress
 * writers of this tree publish other values through the same cell. This
 * target's generated undefined-syms map binds the address, so no map row is
 * added here. */
extern u8 g_ScenarioProgress; /* @source 0x80146864 @kind unknown */

/* Shared frame counter of the main executable, outside this overlay's payload.
 * This handler divides it by the unsigned constant sequence (multu against
 * 0xCCCCCCCD, then a two-bit shift), so the cell is declared unsigned; the
 * shared map and this target's generated undefined-syms map both bind the
 * address. */
extern u32 D_80143E6C; /* @source 0x80143E6C @kind data */

/* Block drawer at 0x801F4E70, still unlifted assembly inside this target, so
 * only its prototype is declared. It draws the four-way block set that the
 * ramp and step handlers of this overlay select with a variant index. */
void func_801F4E70(u32 variant);

/* Drawing pass at 0x801F49A0, still unlifted assembly inside this target; the
 * sibling step handlers run it with pass 0 and pass 1, this handler with pass
 * 2 on the fifth-tick variant path. */
void func_801F49A0(u32 pass);

/* Shared request helper of the main executable, outside this overlay's payload
 * and reached only through this body. Its role is unresolved: the exact lifts
 * that call it treat it as a two-value request helper, so the parameters stay
 * structural. This target's generated undefined-funcs map binds the address, so
 * no map row is added. */
extern void func_8015507C(s32 arg0, s32 arg1); /* @source 0x8015507C */

/* @source 0x801F43EC
 * @behavior Overlay step handler, entry 11 of the work-byte-2 handler table at
 *           0x801F5954, whose word is also entry 14 of the work-byte-1 handler
 *           table at 0x801F5948. While the shared scenario progress byte
 *           g_ScenarioProgress reads 0x0F and the shared frame counter
 *           0x80143E6C divided by five is 3 modulo 4, it hands the block drawer
 *           func_801F4E70 the fixed variant 4, passes the pair 0x43 and 0x1A to
 *           the shared request helper func_8015507C, then increments the work
 *           record's byte 0x02 through one read of the scratchpad pointer cell
 *           0x1F800044 so the next frame dispatches the following entry of the
 *           same table, and returns without running the drawing pass. Every
 *           other frame re-reads the frame counter, divides it by five through
 *           the unsigned constant sequence and, when the counter is an exact
 *           multiple of five, hands the block drawer the current fifth-tick
 *           variant (counter / 5) & 3; it then runs the drawing pass
 *           func_801F49A0(2). Takes no arguments, returns nothing and uses a
 *           0x18-byte frame that keeps only the return address at 0x10. The
 *           roles of the two in-target callees beyond their drawing effects
 *           and of the shared request helper's parameters stay described by
 *           address.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawFourthBlockVariantOnFourthFifthTickGroupAtScenarioProgress0fElseFifthTickVariantAndRunSecondPassWorld02Area10313_801F43EC(
    void) {
  u8* work;
  u32 ticks;
  u32 band;

  if (g_ScenarioProgress == 0xF && ((D_80143E6C / 5) & 3) == 3) {
    func_801F4E70(4);
    func_8015507C(0x43, 0x1A);
    work = D_1F800044;
    work[2]++;
    return;
  }
  ticks = D_80143E6C;
  band = ticks / 5;
  if (ticks == band * 5) {
    func_801F4E70(band & 3);
  }
  func_801F49A0(2);
}
