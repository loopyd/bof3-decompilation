#include "bof3/bof3.h"

extern u8 D_801FD288; /* @source 0x801FD288 @kind data */
extern u8 g_ScenarioProgress;

/* @source 0x801F3018
 * @behavior Leaf overlay helper that tests one byte of shared state and stores
 *           a scenario progress value. It reads the low byte of the packed
 *           scenario word at the cross-overlay address 0x801FD288 with `lbu`
 *           (after that `lbu` the original keeps a `nop`, so the loaded $v0 is
 *           consumed by the very next instruction), subtracts 0x62 and applies
 *           the unsigned bound 5 with `sltiu`, i.e. the loaded byte is tested
 *           against 0x62..0x66. The selected constant is 0x0A when the byte is
 *           inside that window and 0x1E otherwise; both arms materialise it in
 *           $v0, the inside arm fills the `bnez` delay slot, and the single
 *           `sb` after the join writes it to the shared scenario progress byte
 *           0x80146864. Takes no arguments, calls nothing, returns nothing,
 *           writes no other state; leaf with no frame and one exit.
 *
 * Shape evidence: the conditional-expression destination form - a two-constant
 * selection materialised in $v0 with the first constant in the branch delay
 * slot and one shared store - is the exact sibling
 * setStatusFFFFAndScenarioProgress14IfGateElse0AWorld03Area12013_801F42F0
 * (emi/world03/area120/13 0x801F42F0, which stores into this same
 * g_ScenarioProgress byte); this function differs in its test, which is an
 * unsigned window on a just-loaded byte, and in its literals. No caller names
 * this entry: the payload holds no `jal` to 0x801F3018, and its one word
 * reference is entry 10 of the function-pointer run at 0x801F4370 inside this
 * payload's tail blob, so the dispatch index that selects it is unevidenced
 * and the state byte role stays anchored at its address.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setScenarioProgress0AOr1EByScenarioWordByteRange62To66World03Area13613_801F3018(
    void) {
  u8 scenarioByte;

  scenarioByte = D_801FD288;
  g_ScenarioProgress = ((u32)(scenarioByte - 0x62) < 5u) ? 0x0A : 0x1E;
}
