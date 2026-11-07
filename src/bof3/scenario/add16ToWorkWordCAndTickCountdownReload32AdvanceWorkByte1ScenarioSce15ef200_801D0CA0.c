#include "bof3/bof3.h"

/* Published scratch work object: the original materialises this cell with a
 * lui plus an offset load, so the cell is its own fixed-address symbol. The
 * four accesses before the guard share one loaded pointer, while each access
 * of the zero branch loads the cell again because the byte and word stores
 * through the loaded pointer may alias the cell. */
extern u8* D_1F800044; /* @source 0x1F800044 @kind data */

/* @source 0x801D0CA0
 * @behavior Entry 1 of this overlay handler run at 0x801D1B1C, which the
 * overlay entry func_801D0C04 selects with the unsigned work byte at offset
 * +0x1 of the scratchpad work object published at pointer slot 0x1F800044 and
 * calls with no arguments: it advances that object word at +0xC by 0x10 and
 * ticks its countdown byte at +0x9 down by one, both through the one loaded
 * pointer, and the word store the original leaves for the guard branch delay
 * slot. When the decremented byte reaches zero it re-arms that byte with 0x20
 * and advances the same work byte at +0x1 by one, so the next frame dispatches
 * entry 2 (0x801D0D00). It takes no arguments, returns nothing and keeps no
 * frame.
 *
 * Evidence: the fourteen words from 0x801D1B1C name 0x801D0C70, 0x801D0CA0,
 * 0x801D0D00, 0x801D0D5C, 0x801D0DE8, 0x801D0E30, 0x801D0EC4, 0x801D0F68,
 * 0x801D0F9C, 0x801D1008, 0x801D1050, 0x801D10E8, 0x801D1118 and 0x801D11A4,
 * so this body is element 1 of that run. The byte at +0x9 is the countdown the
 * exact lift tickCountdownReload16AndAdvanceWorkByte1ScenarioSce15ef200_801D0D00
 * ticks down and re-arms, and the word at +0xC is the accumulator that this
 * overlay resets in func_801D0C70 and ramps by -0x10 in func_801D0D5C.
 *
 * Shape evidence: the neighbouring span func_801D0D5C repeats this body word
 * for word with -0x10 instead of 0x10 and a call where the re-arm stands, and
 * its 0x48-byte size rules the span out as a clone. The exact lift
 * decrementWorkByte9AdvanceWorkWord14By20ThenDecrementGlobalWorkByteBAndInvokeHelperWhenZeroBmagicMagic15103_801F1D60
 * (0x801F1D60 in emi/bmagic/magic151/03) records the same local-pointer word
 * advance whose store lands in the guard branch delay slot, the same
 * `andi $v1,$v1,0xFF` guard on the stored byte, and a body that re-reads its
 * own pointer cell.
 * @status exact
 * @match 100.00
 * @residual none
 */
void add16ToWorkWordCAndTickCountdownReload32AdvanceWorkByte1ScenarioSce15ef200_801D0CA0(
    void) {
  u8* work;
  s32 value;

  work = SPAD_PTR_SLOT(u8, 0x44u);
  value = *(s32*)(work + 0xC) + 0x10;
  *(s32*)(work + 0xC) = value;
  if (--work[0x9] == 0) {
    D_1F800044[0x9] = 0x20;
    D_1F800044[0x1]++;
  }
}
