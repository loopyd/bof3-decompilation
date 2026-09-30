#include "bof3/bof3.h"

extern int rand(void);

extern u16 D_801FD288; /* @source 0x801FD288 @kind data */

/* @source 0x801F71FC
 * @behavior Overlay per-frame work handler and entry 0 of the code-pointer run
 * based at 0x801FD184 (the word at 0x801FD184), which is the run the byte-2
 * dispatcher dispatchWorkByte2HandlerScenarioScena1200_801F71B8 selects with the
 * work object's byte at offset 0x02. It stores (rand() & 1) << 15, that is
 * 0x8000 or 0x0000, into the overlay-local halfword D_801FD288: bit 15 of that
 * halfword is the flag the next handler of the same run, func_801F7240, shifts
 * down to select its phase-count threshold (`srl $s2, $v1, 15`), keeps in the
 * repacked halfword (`sll $v0, $s2, 15`) and the handler also reads the
 * halfword's low byte as a 0..0xC7 counter; func_801F75EC and func_801F9680 read
 * that low byte back with `lbu`. After that it reads the scratchpad work object
 * published by the pointer cell at 0x1F800044, takes the object's unsigned byte
 * at offset 0x02, adds one and stores it back with `sb 0x2`, so it advances the
 * byte-2 state index of the window this run is selected by. It takes no
 * arguments, returns nothing and touches no other state; the pointer cell is
 * loaded after the rand() call at 0x8017E3D4 and the 0x18-byte frame only keeps
 * $ra across that call. The name records the proven behavior — seed the random
 * sign bit of D_801FD288, then advance the work object's byte 2 — and the
 * address anchor keeps it target-qualified.
 * @status exact
 * @match 100.00
 * @residual none
 */
void seedRandomSignAndAdvanceWorkByte2ScenarioScena1200_801F71FC(void) {
  u8 *work;

  D_801FD288 = (rand() & 1) << 15;
  work = SPAD_PTR_SLOT(u8, 0x44u);
  work[2]++;
}
