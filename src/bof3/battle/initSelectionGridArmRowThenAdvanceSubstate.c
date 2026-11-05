#include "bof3/battle/battle15_internal.h"

/* @source 0x80096FBC
 * @behavior Confirm-selection substate step, index 1 of the 0x800B43EC handler
 *   table: stores 4 into byte +0x01 of the active selection slot pointer, then
 *   re-initialises the primary selection grid at 0x80148570 through
 *   func_80097EC0, stores 2 into the grid row byte 0x8014857B while bit 1 of the
 *   slot flag word +0x10 is set and bit 17 is clear, and finally advances the
 *   battle substate index 0x801462E3, clears its own confirm-substate index
 *   0x801462E4 and mirrors the active message-slot work byte +0x05 into the
 *   work-block byte 0x80148656.
 * @status exact
 * @match 100.00
 * @residual none
 * The flag store needs two separate mask tests: the equivalent `&&` spelling
 * folds to the single `(flags & 0x20002) == 2` test, which the original does not
 * have. The slot pointer cell is reloaded after the call and read through its
 * own expression, so the reloaded value takes $v0 and the flag word $v1 as the
 * original does.
 */
void initSelectionGridArmRowThenAdvanceSubstate(void) {
  u8 *active;
  u8 *work;
  u32 flags;
  u8 counter;
  u8 value;

  active = D_801EB4D8;
  active[1] = 4;
  func_80097EC0();
  flags = *(u32 *)(D_801EB4D8 + 0x10);
  if ((flags & 2u) != 0u) {
    if ((flags & 0x20000u) == 0u) {
      D_8014857B = 2;
    }
  }
  work = D_801EBF08;
  counter = D_801462E3;
  value = work[5];
  D_801462E4 = 0;
  D_801462E3 = counter + 1;
  D_80148656 = value;
}
