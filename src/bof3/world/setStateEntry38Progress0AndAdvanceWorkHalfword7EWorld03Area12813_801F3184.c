#include "bof3/bof3.h"

/* @source 0x8015C088 @kind function */
extern void func_8015C088(void);
/* @source 0x801448EB @kind data */
extern u8 D_801448EB;
/* @source 0x801448EC @kind data */
extern u8 D_801448EC;

/* @source 0x801F3184
 * @behavior Calls the shared front-end startup helper at 0x8015C088, writes
 *           0x38 to the shared state-entry byte at 0x801448EB, clears the
 *           progress byte at 0x801448EC, and increments the unsigned halfword
 *           at +0x7E of the supplied work record. Returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setStateEntry38Progress0AndAdvanceWorkHalfword7EWorld03Area12813_801F3184(
    u8* work) {
  func_8015C088();
  D_801448EB = 0x38;
  D_801448EC = 0;
  *(u16*)(work + 0x7E) += 1;
}
