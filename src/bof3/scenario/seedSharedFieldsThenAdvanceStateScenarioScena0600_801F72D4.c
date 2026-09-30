#include "bof3/bof3.h"

extern s8 D_80146872;

void func_801FD29C(void);

/* @source 0x801F72D4
 * @behavior Entry 0 of this overlay's 35-pointer frame-state handler table at
 * 0x801FE3D8 (0x801F72D4, 0x801F7300, 0x801F7A00, ..., 0x801FD234), the table
 * the overlay dispatcher func_801F7298 indexes with the signed main-RAM state
 * byte D_80146872: it first calls func_801FD29C, which copies the overlay's
 * eight-byte constant block at 0x801FE464 (0x32, 0x32, 0x1E, 0x18, 0x00, 0x0C,
 * 0x00, 0x00) into the shared main-RAM halfword fields 0x80144DF8, 0x80144E00,
 * 0x80144E04, 0x80144E06, 0x80144E08, 0x80144E0A and their 0x20-mirror bank
 * 0x80144E20, 0x80144E24, 0x80144E26, 0x80144E28, 0x80144E2A plus the shared
 * bytes 0x80144DF2/0x80144DF5, and then writes 1 to D_80146872 so the next
 * frame dispatches entry 1 (0x801F7300). Takes no arguments and returns
 * nothing; the constant 1 sits in $v0 only because the state byte store needs a
 * register operand.
 * @status exact
 * @match 100.00
 * @residual none
 */
void seedSharedFieldsThenAdvanceStateScenarioScena0600_801F72D4(void) {
  func_801FD29C();
  D_80146872 = 1;
}
