#include "bof3/bof3.h"

extern s8 D_801490C7;
extern u16 D_801490A8;
extern u8 D_801D4286;

/* @source 0x801F34A8
 * @behavior Loads the shared gate byte at 0x801490C7 with a signed byte read
 *           (`lb`, so the cell is s8); while that byte is nonzero it stores the
 *           constant 3 into the handler index byte at 0x801D4286 (`addiu
 *           v0,0,3` in the branch delay slot, `sb`), and it then stores 0xFFFF
 *           into the shared status halfword at 0x801490A8 (`ori v0,0,0xFFFF`,
 *           `sh`) on every path. No arguments, no return value; the only writes
 *           are the conditional 0x801D4286 store and the unconditional
 *           0x801490A8 store.
 *           Constant/role evidence: one payload `cmp` over the two 48-byte
 *           spans found 0x801F34A8..0x801F34D8 equal to this target's sibling
 *           setStatusFFFFAndHandlerIndex5WhenGateWorld00Area03713_801F34D8 in
 *           every byte but the branch-delay index immediate (3 here, 5 there),
 *           so both are one body whose stored handler index separates them,
 *           and the 0x801F34A8 word is the second entry of the four five-word
 *           handler rows at 0x801F4A00, 0x801F4A14, 0x801F4A28 and 0x801F4A3C,
 *           which indexes the span as a step handler. The same 48 bytes sat in
 *           thirteen other world overlay payloads; at the time of writing two of
 *           those copies were wired as closures carrying a 100.00 gate result
 *           (setStatusFFFFAndHandlerIndex3WhenGateWorld02Area09113_801F2C84 and
 *           the area041 copy at 0x801F3CD4).
 * @status exact
 * @match 100.00
 * @residual none
 */
void setStatusFFFFAndHandlerIndex3WhenGateWorld00Area03713_801F34A8(void) {
  if (D_801490C7 != 0) {
    D_801D4286 = 3;
  }
  D_801490A8 = 0xFFFF;
}
