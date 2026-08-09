#include "bof3/ui/game01_internal.h"

/* @behavior when fade phase `2` is reached, arms a 360-tick delay and advances
 * the frontend state.
 * @source 0x801D0D5C
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void armFadeDelay(void) {
  /* MATCHING_AID: read the fade-phase byte through a non-volatile view.
   * cc1 (gcc-2.7.2-psx) emits an explicit zero-extension (andi 0xff after the
   * lbu) whenever a *volatile* narrow load feeds a non-zero comparison, and that
   * in-place andi pins the value to $v0, forcing the constant 2 into $v1 (a
   * register swap). The original binary has no mask: lbu $v1 / li $v0,2 / bne.
   * Casting away volatile lets cc1 absorb the zero-extension into the lbu and
   * reproduces that register allocation. Remove if GAME_FRONT_FADE_PHASE is ever
   * re-declared non-volatile. */
  if (*(u8*)&GAME_FRONT_FADE_PHASE == 2u) {
    /* MATCHING_AID: stage the body through ordered temporaries around a
     * barrier() to reproduce the original schedule:
     *   bne ...; li $v1,360 (delay); lhu $v0,STATE; sh $v1,TIMER; addiu; sh.
     * Holding 360 in `timer` and STATE in `state` makes cc1 allocate 360->$v1
     * and the STATE word->$v0, and reading STATE before storing TIMER hoists the
     * lhu ahead of the sh so the sh fills the lhu load-delay slot (no nop).
     * The barrier() keeps the `li $v1,360` ahead of the STATE load so the reorg
     * pass places it in the bne delay slot; without it cc1 schedules the lhu
     * first and strands `li $v1,360` in the lhu load-delay slot, leaving a nop
     * in the bne delay slot. Remove if cc1 ever defers the constant load into
     * the branch delay slot on its own. */
    u16 state;
    u16 timer = 360u;
    barrier();
    state = GAME_FRONT_STATE;
    GAME_FRONT_TIMER = timer;
    GAME_FRONT_STATE = state + 1u;
  }
}
