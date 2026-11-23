#include "bof3/bof3.h"

/* Shared front-end state selector of the main executable: the dispatcher
 * dispatchIndexedStateHandler indexes the handler table D_801C84BC with it.
 * @source 0x801448EB @kind unknown
 */
extern s8 D_801448EB;

/* Neighbouring shared byte of the same three-byte group, cleared together with
 * the selector above; its role is not established here.
 * @source 0x801448ED @kind unknown
 */
extern s8 D_801448ED;

/* Shared front-end startup helper of the shared EXE image, outside this
 * payload: it takes no arguments and its answer, if any, is not used here.
 */
void func_8015C088(void);

/* @source 0x801F3C9C
 * @behavior Overlay step handler the loader reaches from outside this payload:
 *           no `jal` immediate and no pointer word in the payload names
 *           0x801F3C9C. It runs the shared front-end startup helper
 *           func_8015C088 with no arguments, then writes 0x3F into the shared
 *           front-end state selector D_801448EB, which
 *           dispatchIndexedStateHandler indexes into the handler table
 *           D_801C84BC, and clears the shared byte D_801448ED, leaving the
 *           middle byte 0x801448EC untouched. It takes no arguments and
 *           returns nothing: the 0x18-byte frame only keeps $ra across the
 *           call, the delay slot behind that jal stays a nop because the
 *           helper takes no arguments, the store constant is materialised into
 *           $v0 because the body sets no return value, and each byte store
 *           materialises the 0x8014 page in $at on its own. The neighbour
 *           handler at 0x801F3CD0 clears the same two shared bytes from its
 *           second step.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setupFrontendClearSecond(
    void) {
  func_8015C088();
  D_801448EB = 0x3F;
  D_801448ED = 0;
}
