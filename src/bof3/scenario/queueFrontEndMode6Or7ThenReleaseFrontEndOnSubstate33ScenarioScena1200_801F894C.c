#include "bof3/bof3.h"

/* Shared sub-state byte of the main executable: the body reads it once through
 * a pointer it keeps in $s0 and stores the next sub-state through the same
 * register.
 * @source 0x80146875 @kind unknown
 */
extern u8 D_80146875;

/* Shared front-end mode byte of the main executable: both front-end arms arm it
 * with 2 and the completion arm returns untouched while it reads 2.
 * @source 0x80143BB0 @kind unknown
 */
extern u8 D_80143BB0;

/* Shared state byte of the main executable: the completion arm clears it, which
 * returns the overlay to its idle handler.
 * @source 0x80146874 @kind unknown
 */
extern u8 D_80146874;

/* Front-end mode request of the main executable at 0x80150224, outside this
 * payload: the body sets nothing but $a0 for it. */
void func_80150224(s32 arg0);

/* Shared front-end release helper at 0x8015C058, outside this payload. */
void func_8015C058(void);

/* @source 0x801F894C
 * @behavior Overlay secondary-state handler: it is the word at 0x801FD1CC,
 * entry 2 of the ten-word handler run at 0x801FD1C4 that
 * dispatchSecondaryStateScenarioScena1200_801F886C (0x801F886C) indexes with the
 * signed shared state byte D_80146874, so it runs while that byte reads 2. That
 * table word is the only little-endian word of the payload equal to this address
 * and no jal or j in the payload targets it. The body steps the shared sub-state
 * byte D_80146875 through one front-end excursion: it reads that byte once,
 * through an address taken into a local pointer and therefore held in $s0 for
 * the whole body. Sub-state 0x32 requests front-end mode 6 and sub-state 0x3C
 * requests front-end mode 7, both through the shared front-end request
 * func_80150224 (0x80150224, outside this payload), and both arms then store 2
 * in the shared front-end mode byte D_80143BB0 and advance the sub-state byte to
 * 0x33; the two arms end in the same call-and-store tail, so each mode id
 * reaches $a0 from the delay slot of that case test and the shared tail follows
 * the taken branch. Sub-state 0x33 tests D_80143BB0 and returns with every cell
 * untouched while that byte still reads 2; once it has left 2 the arm runs the
 * shared front-end release helper func_8015C058 (0x8015C058) and clears the
 * sub-state byte and the state byte D_80146874, which returns the overlay to the
 * idle entry 0 of the same handler run. Any other sub-state value returns
 * untouched. It takes no arguments, returns nothing and writes no cell beyond
 * those four stores. Its 0x18-byte frame keeps only $ra and $s0; the three case
 * values compile to one balanced test with the 0x33 pivot and the unsigned bound
 * 0x34, the comparison of D_80143BB0 reads it with lbu, and the state-byte store
 * of the completion arm materialises the 0x8014 page in $at on its own. Its 176
 * bytes have no byte-identical copy in the tree: a scan of every payload under
 * out/binaries that compares all 44 words with only the j and jal targets masked
 * matched this span alone, so the body was seeded from the statement order, cell
 * names and declaration style of the exact same-target sibling
 * setProgress50AndAdvanceSubstate0BUnlessFrontEndMode2ThenReleaseFrontEndWhenSharedByte65Reads0AScenarioScena1200_801F88B0
 * (0x801F88B0, entry 1 of the same handler run) and of the exact sibling
 * queueFrontEndMode3EThenWaitModeLeave2ReleaseAndClearStateScenarioScena0200_801F8264
 * (0x801F8264 of emi/scenario/scena02/00), writing this span's own case values,
 * mode ids and store order from its own bytes.
 * @status exact
 * @match 100.00
 * @residual none
 */
void queueFrontEndMode6Or7ThenReleaseFrontEndOnSubstate33ScenarioScena1200_801F894C(void) {
  u8 *substate;

  substate = &D_80146875;
  switch (*substate) {
  case 0x32:
    func_80150224(6);
    D_80143BB0 = 2;
    *substate = 0x33;
    break;
  case 0x33:
    if (D_80143BB0 == 2) {
      break;
    }
    func_8015C058();
    *substate = 0;
    D_80146874 = 0;
    break;
  case 0x3C:
    func_80150224(7);
    D_80143BB0 = 2;
    *substate = 0x33;
    break;
  }
}
