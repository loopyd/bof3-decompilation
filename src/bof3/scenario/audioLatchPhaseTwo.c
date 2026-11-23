#include "bof3/bof3.h"

/* Shared front-end phase byte of the main executable: sub-state 0 latches this
 * byte to 2 and a later sub-state holds until it stops reading 2, which is the
 * store-then-observe use the scenario overlays make of the cell.
 * @source 0x80143BB0 @kind data
 */
extern u8 D_80143BB0;

/* Shared primary scenario state byte of the main executable.
 * @source 0x80146874 @kind data
 */
extern u8 D_80146874;

/* Shared scenario sub-state byte of the main executable.
 * @source 0x80146875 @kind data
 */
extern u8 D_80146875;

/* Shared scenario countdown halfword of the main executable, which the same
 * scenario family loads, decrements and re-arms (0x1E, 0x20, 0x130, 0x200).
 * @source 0x80146876 @kind data
 */
extern u16 D_80146876;

/* Shared front-end action helper of the main executable, outside this payload:
 * it starts the action its argument ids.
 * @source 0x80150224 @kind function
 */
extern void func_80150224(s32 arg0);

/* Shared front-end startup helper of the main executable, outside this payload.
 * Declared void in this translation unit because the shipped bytes materialise
 * both stored constants after the call in $v0, so no result is live across
 * those stores; the sibling requestPrimaryArm
 * carries the same three stores in $v1, which is what its value-returning
 * declaration produces there.
 * @source 0x8015C088 @kind function
 */
extern void func_8015C088(void);

/* @source 0x801FCE18
 * @behavior Word 0 of the overlay's four-word no-argument handler run at 0x801FD038
 * ({0x801FCE18, 0x801FCE6C, 0x801FCEDC, 0x801FCEFC}), which
 * dispatchRoute63Lookup (0x801FCDB4)
 * indexes with the signed-byte answer of the resident route lookup
 * func_801A7AD8 and calls after its gate, discarding the handler's answer. That
 * pointer word is this span's only in-image reference: no `jal` or `j` word of
 * the payload targets 0x801FCE18, so it takes no arguments.
 *
 * It starts the front-end action 0x19 through the shared action helper
 * func_80150224, latches the shared front-end phase byte D_80143BB0 to 2, runs
 * the shared front-end startup helper func_8015C088, and then requests primary
 * scene state 7 by storing 7 in the shared state byte D_80146874, clears the
 * shared sub-state byte D_80146875 and arms the shared countdown D_80146876
 * with 0x1E (30) in the original order 0x1E, then 7, then 0. It answers
 * nothing: the body ends on `jr $ra` with no value setup, and the 7 that the
 * last store leaves in $v0 is that store's own constant, not a returned value.
 *
 * The 84 bytes use a 0x18-byte frame that keeps only $ra across the two calls,
 * every shared store re-materialises the 0x8014 page in $at on its own, and the
 * zero store writes $zero directly, so all four stored values are literals.
 * No byte-identical copy of this span exists in the 398 extracted payloads
 * under out/binaries: the plain 84-byte scan and the scan with every `j`/`jal`
 * immediate masked both hit only this span's own offset, so clone was declined
 * and the body was written from this span's own bytes. The trailing three-store
 * group and its operand form come from the same target's exact sibling
 * requestPrimaryArm (0x801FC6C4),
 * and the leading action-then-latch pair from the exact sibling
 * startAction34LatchPhaseByteTwoThenReleaseFrontEndAndClearPrimaryStateScenarioScena0100_801FCE30
 * (0x801FCE30 of emi/scenario/scena01/00); this span stores no constant either
 * sibling stores.
 * @status exact
 * @match 100.00
 * @residual none
 */
void audioLatchPhaseTwo(void) {
  func_80150224(0x19);
  D_80143BB0 = 2;
  func_8015C088();
  D_80146876 = 0x1E;
  D_80146874 = 7;
  D_80146875 = 0;
}
