#include "bof3/bof3.h"

/* Shared front-end phase byte of the main executable: sub-state 0 latches this
 * byte to 2 and sub-state 1 holds until it stops reading 2, which is the
 * store-then-wait use the scenario overlay sequences make of the cell.
 * @source 0x80143BB0 @kind data
 */
extern u8 D_80143BB0;

/* Shared primary scenario state byte of the main executable: the closing store
 * of 6 requests primary state 6 and ends the chain this handler belongs to.
 * @source 0x80146874 @kind data
 */
extern u8 D_80146874;

/* Shared scenario sub-state byte of the main executable and the switch selector
 * of this body.
 * @source 0x80146875 @kind data
 */
extern u8 D_80146875;

/* Shared front-end action helper of the main executable, outside this payload:
 * it starts the action its argument ids.
 * @source 0x80150224 @kind function
 */
extern void func_80150224(s32 arg0);
/* Shared front-end release helper of the main executable, outside this payload.
 * @source 0x8015C058 @kind function
 */
extern void func_8015C058(void);

/* @source 0x801FCD9C
 * @behavior Two-step handler over the shared sub-state byte D_80146875
 * (0x80146875): it takes that byte's address once into a local pointer, reads it
 * with `lbu` once as the switch selector and runs one of two sequences over it.
 * Sub-state 0 starts action 0x34 through the shared action helper func_80150224
 * (0x80150224), latches the shared front-end phase byte D_80143BB0
 * (0x80143BB0) to 2 and advances the sub-state byte to 1. Sub-state 1 returns
 * while that phase byte still reads 2 and, once it differs, runs the shared
 * front-end release helper func_8015C058 (0x8015C058), clears the sub-state byte
 * and requests primary scenario state 6 by storing 6 in the shared primary state
 * byte D_80146874 (0x80146874). Any other sub-state value returns without
 * touching any byte. It takes no arguments and returns nothing, and its
 * 0x18-byte frame keeps $ra and $s0, the latter because the pointer to the
 * sub-state byte stays live across both calls; the `lui`/`addiu` pair that forms
 * that pointer is scheduled between the two prologue saves.
 *
 * The payload refers to 0x801FCD9C through exactly one word, the pointer at
 * 0x801FE2FC: no `jal` or `j` immediate targets it, and that word is entry 24 of
 * the contiguous code-pointer run based at 0x801FE29C that the exact dispatcher
 * callProgressTableHandler (0x801F7FC4) indexes with
 * the shared scenario progress byte D_80146872.
 *
 * No byte-identical copy of these 0x94 bytes exists in any other of the 396 EMI
 * payloads under `out/binaries/emi`, and no other payload holds a window equal
 * to them in every word that is not an absolute `j`/`jal` immediate, so the
 * clone lever is declined on measurement. The body was seeded from the
 * in-target exact sibling
 * audioStartAction34
 * (0x801FCE30), which carries the same statement order, the same saved
 * sub-state pointer and the same delay-slot constants 1 and 2 and differs only
 * in the closing store: 0 there, 6 here, which costs this body the one extra
 * `addiu $v0, $zero, 0x6` before the sub-state clear and the `sb $v0` instead of
 * `sb $zero` into D_80146874. The four relocation-different scena05 twins that
 * sibling records are its shape evidence and store 0 as well, so none of them is
 * clone-eligible for this span.
 * @status exact
 * @match 100.00
 * @residual none
 */
void audioLatchPhaseRelease(void) {
  u8 *substate = &D_80146875;

  switch (*substate) {
  case 0:
    func_80150224(0x34);
    D_80143BB0 = 2;
    *substate = 1;
    break;
  case 1:
    if (D_80143BB0 != 2) {
      func_8015C058();
      *substate = 0;
      D_80146874 = 6;
    }
    break;
  }
}
