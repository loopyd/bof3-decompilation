#include "bof3/bof3.h"

/* Scratchpad work-record pointer cell. Naming the cell keeps the original's
 * per-access re-materialization (lui $vN,0x1F80 / lw $vN,68($vN)); the bare
 * constant address makes the compiler CSE 0x1F800044 into one saved register
 * and load through offset zero instead.
 * @source 0x1F800044 @kind data
 */
extern u8* D_1F800044;

/* Shared overlay state word. This handler compares the whole word against a
 * packed pair of halfwords; other scenes write single small values through the
 * leading halfword, so the word load belongs to this target's own use.
 * @source 0x80143B90 @kind data
 */
extern u32 D_80143B90;

/* Status halfword whose 0x4000 bit arms the step advance below. The halfword
 * access follows the original's lhu; the other bits it carries stay described
 * by address.
 * @source 0x801EB7CA @kind data
 */
extern u16 D_801EB7CA;

/* Block drawer at 0x801F4E70, still unlifted assembly inside this target, so
 * only its prototype is declared. It draws the four-way block set that the
 * ramp and step handlers of this overlay select with a variant index. */
void func_801F4E70(u32 variant);

/* @source 0x801F457C
 * @behavior Overlay step handler, entry 13 of the work-byte-2 handler table at
 *           0x801F5954. While the shared overlay state word at 0x80143B90
 *           holds 0x00050005 and the status halfword at 0x801EB7CA has its
 *           0x4000 bit set, it advances the work record's byte 0x02 through a
 *           fresh read of the scratchpad pointer cell 0x1F800044, so the next
 *           frame dispatches entry 14 of the same table. It then rewinds the
 *           record's byte 0x09 to zero when that byte stands at 0x0F and, on
 *           every tick whose byte 0x09 is a multiple of five, hands the block
 *           drawer func_801F4E70 the band index (byte 0x09 / 5) + 7. A final
 *           cell read advances byte 0x09 by one. Takes no arguments, returns
 *           nothing and uses a 0x18-byte frame that holds only the saved return
 *           address at 0x10. The roles of the state word, the status halfword
 *           and the callee beyond those uses stay described by address.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawWorkBandOnFifthTickWrapWorkByte9At0fThenAdvanceWorkByte9World02Area10313_801F457C(
    void) {
  u8* work;
  u8* bandCursor;
  u8* tickCursor;
  u8* stepCursor;
  u32 ticks;

  if (D_80143B90 == 0x00050005u && (D_801EB7CA & 0x4000) != 0) {
    work = D_1F800044;
    work[2]++;
  }
  bandCursor = D_1F800044;
  if (bandCursor[9] == 0x0F) {
    bandCursor[9] = 0;
  }
  tickCursor = D_1F800044;
  ticks = tickCursor[9];
  if ((u8)(ticks % 5) == 0) {
    func_801F4E70((u8)(ticks / 5) + 7);
  }
  stepCursor = D_1F800044;
  stepCursor[9]++;
}
