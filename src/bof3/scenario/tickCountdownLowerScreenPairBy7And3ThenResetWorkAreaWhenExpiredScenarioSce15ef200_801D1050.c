#include "bof3/bof3.h"

/* Published scratchpad work object: the original materialises this cell with a
 * lui plus an offset load, so the cell is its own fixed-address symbol. This
 * body keeps one loaded pointer for all four accesses, so it needs no reload. */

/* Shared work-area reset helper of the game image: it zeroes work-object bytes
 * 0x00-0x04, including the work byte this overlay dispatches on, so the overlay
 * drops back to work state 0. No shipped header declares it, so the prototype
 * stays local, exactly as in the sibling lift of the same helper. */
void func_80196070(void);

/* @source 0x801D1050
 * @behavior Entry 10 of this overlay's fourteen-word code-pointer run at
 * 0x801D1B1C, which the overlay entry func_801D0C04 selects with the unsigned
 * work byte at offset +0x1 of the scratchpad work object published at pointer
 * slot 0x1F800044 and calls with no arguments: through the one loaded pointer
 * it lowers that object's projected screen-pair halfwords at +0x2E and +0x30 by
 * 7 and 3, ticks its countdown byte at +0x9 down by one, and leaves the +0x30
 * store for the guard branch delay slot. When the decremented byte reaches zero
 * it resets the shared work area through func_80196070, which zeroes
 * work-object bytes 0x00-0x04 including that dispatch byte, so the overlay
 * drops back to work state 0. It takes no arguments, returns nothing and its
 * 0x18-byte frame only keeps $ra at 0x10 across that call.
 *
 * Evidence: the fourteen words from 0x801D1B1C name 0x801D0C70, 0x801D0CA0,
 * 0x801D0D00, 0x801D0D5C, 0x801D0DE8, 0x801D0E30, 0x801D0EC4, 0x801D0F68,
 * 0x801D0F9C, 0x801D1008, 0x801D1050, 0x801D10E8, 0x801D1118 and 0x801D11A4,
 * so this body is element 10 of that run, the entry the run's own step
 * dispatchers reach through word 10. The +0x9 byte is the countdown that
 * entries 1, 2, 7, 8 and 12 of the same run tick and re-arm, the halfwords at
 * +0x2E and +0x30 are the reported x and y that the run's draw calls pass to
 * func_801D17A0, and func_80196070 is the work-area reset the run's entry 6
 * (stepWorkTableValueF0AndResetWorkStateWhenNoRecordActive) also calls, so the
 * zero arm ends this overlay's work state rather than re-arming the countdown.
 *
 * Shape: the guard-and-reset tail repeats the exact lift
 * tickCountdownMoveScreenPairReload90AndAdvanceWorkByteScenarioSce15ef200_801D0F9C
 * of this target - the load of the work pointer, the 0x18-byte frame inserted
 * after it, the local ticked countdown byte, the two halfword moves and the
 * store left for the guard branch delay slot - with the reset call in place of
 * that body's re-arm and work-byte advance. A raw scan of all 396 extracted
 * payloads under out/binaries finds this 0x54-byte span only in this target,
 * and a repeat with the `jal` immediate masked finds no other twin, so the span
 * is not a clone.
 * @status exact
 * @match 100.00
 * @residual none
 */
void tickCountdownLowerScreenPairBy7And3ThenResetWorkAreaWhenExpiredScenarioSce15ef200_801D1050(
    void) {
  u8* work;
  u8 count;
  u16 x;
  u16 y;

  work = SPAD_PTR_SLOT(u8, 0x44u);
  count = work[0x9] - 1;
  x = *(u16*)(work + 0x2E) - 7;
  y = *(u16*)(work + 0x30) - 3;
  work[0x9] = count;
  *(u16*)(work + 0x2E) = x;
  *(u16*)(work + 0x30) = y;
  if ((count & 0xFFu) == 0) {
    func_80196070();
  }
}
