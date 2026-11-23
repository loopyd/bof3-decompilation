#include "bof3/bof3.h"

extern u32 D_8014686C;

void func_8015B580(u32 arg0, s32 arg1);

/* @source 0x801FC810
 * @behavior Thin overlay wrapper setting flag bit 0x14 in the active scenario
 * flag word D_8014686C through the shared bit-set helper at 0x8015B580; it
 * loads that word and passes it as the first argument with 0x14 as the second,
 * takes no arguments and returns nothing, and its 0x18-byte frame only keeps
 * $ra across the call. No in-image jal targets the address: its only image
 * word is 0x801FE45C, word 10 of the 16-word handler table at 0x801FE434 that
 * the overlay dispatcher func_801FC5E4 indexes with byte 0x7A of a record
 * (passing the shared scenario flag word D_8014686C in the second argument
 * register), so the overlay reaches this handler only indirectly. Its 0x28
 * bytes differ from the same-target sibling setFlagBit10
 * (0x801FC7E8) only in the stored flag-bit constant, and they equal the scena06
 * sibling setScenarioFlagBit0CScenarioScena0600_801FCF98 (0x801FCF98) apart
 * from that same constant.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setFlagBit14(void) {
  func_8015B580(D_8014686C, 0x14);
}
