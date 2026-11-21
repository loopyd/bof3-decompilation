#include "bof3/bof3.h"

/* Shared record pointer cell at 0x80148210. Its +0x0A halfword is the countdown
 * every step of this overlay's handler run subtracts 2 from; the original loads
 * the cell once with one lw and stores through the loaded pointer with one sh.
 * @source 0x80148210 @kind unknown
 */
extern u16* D_80148210;

/* Local step helper at 0x801F3140. The four handlers of the 0x801F4580 run call
 * it with -4, -3, -2 and -1, so step 3 passes -1. */
void func_801F3140(s16 step_code);

/* @source 0x801F3108
 * @behavior Step 3 of this overlay's four-entry handler run based at 0x801F4580,
 * which dispatchStateByte4Table4580World03Area14913_801F2EFC indexes with work
 * byte 4. The body has no gate of its own: it calls the local step helper
 * func_801F3140 with -1 and then subtracts 2 from the +0x0A halfword of the
 * record published through the pointer cell 0x80148210. That is the same tail
 * the step 0 and step 2 bodies at 0x801F2F40 and 0x801F3068 carry in the
 * original bytes, with -4 and -2 in the helper's delay slot. The 0x18-byte frame
 * keeps $ra at +0x10, the original's two load delay slots stay nops, and the
 * $ra reload lands between the -2 and the store. Takes no arguments and returns
 * nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void invokeHelperAndDecrementRecordHalfword0aWorld03Area14913_801F3108(void) {
  func_801F3140(-1);
  D_80148210[5] -= 2;
}
