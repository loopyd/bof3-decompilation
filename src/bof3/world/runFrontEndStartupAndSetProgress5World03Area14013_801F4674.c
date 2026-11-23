#include "bof3/bof3.h"

/* Shared front-end startup helper of the main executable, outside this
 * payload. Takes no arguments and returns nothing.
 * @source 0x8015C088 */
extern void func_8015C088(void);

/* Progress byte of the shared state-entry tuple 0x801448EB..0x801448ED, which
 * the sibling world handlers of this family write with one `sb` per arm.
 * @source 0x801448EC @kind data */
extern u8 D_801448EC;

/* @source 0x801F4674
 * @behavior Overlay entry that starts one front-end step unconditionally: it
 *           runs the shared front-end startup helper func_8015C088 and stores
 *           progress 5 into the shared progress byte D_801448EC, with the
 *           stored value left in $v0. The 0x18-byte frame saves only $ra.
 *
 * Name evidence: the body is the gate-free arm of the sibling progress
 * request setProgress5OnFlag66AndPackedArgWindowMatchWorld03Area14013_801F42F4,
 * which reaches the same two statements (func_8015C088 then progress 5 into
 * D_801448EC) after its packed-word and flag gates, so this overlay's own row
 * is the unconditional form of that request. No image word of this target
 * holds 0x801F4674 and no jal in the payload targets it, so the body is
 * reached by the overlay loader and takes no arguments.
 * @status exact
 * @match 100.00
 * @residual none
 */
void runFrontEndStartupAndSetProgress5World03Area14013_801F4674(void) {
  func_8015C088();
  D_801448EC = 5;
}
