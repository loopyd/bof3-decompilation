#include "bof3/bof3.h"

/* Flag bank pointer of the shared main executable. The shared scenario loader
 * at 0x801A7704 publishes it as the scenario table D_80144E88 plus the current
 * scenario id times 8, so it addresses the active scenario's flag bank. This
 * overlay's neighbouring flag wrappers read the same word and hand it to the
 * shared bit setter and clearer.
 * @source 0x8014686C @kind unknown
 */
extern volatile u32 D_8014686C;

/* Shared bit setter of the main executable: writes the bit its second argument
 * selects in the flag bank its first argument addresses.
 */
extern void func_8015B5F4(void* flag_bank, u8 bit_index);

/* @source 0x801F30A8
 * @behavior Sets bit 0x1F of the active scenario flag bank with one call to
 * the shared bit setter func_8015B5F4, handing it the pointer published at
 * 0x8014686C and the bit index 0x1F. Takes no arguments and returns nothing;
 * the 0x18-byte frame keeps only $ra at +0x10.
 *
 * Shape evidence: this is the same one-bit wrapper the scenario flag helpers of
 * this tree already lift — setScenarioFlagBit28 and setScenarioFlagBit2B pass
 * the same cell to the shared setter, and clearScenarioFlagBit28 passes it to
 * the shared clearer, each as a single call over the pointer cell. Those
 * sources supplied the pointer-local spelling `(void*)D_8014686C` and the call
 * order; the callee and the bit index here are written from this span's own
 * bytes: `jal` to 0x8015B5F4 rather than the setter 0x8015B580 or the clearer
 * 0x8015B5A8, and `addiu $a1, $zero, 0x1F` rather than 0x28 or 0x2B.
 *
 * Clone declined: a scan of all 398 extracted payloads under out/binaries for
 * this 0x28-byte body returned only this span's own offset in its own payload,
 * so no byte-identical sibling exists. The family members differ in their bit
 * constant and in which of the adjacent setter/clearer words they call, so none
 * of them can be byte-identical either.
 *
 * The caller is unnamed: no `jal` and no word of this payload holds this span's
 * address, which appears only as entry 5 of the overlay's exported pointer run
 * at 0x801F492C (word 0x801F4940), so the argument roles stay structural.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateScenarioFlagBit1(void) {
  func_8015B5F4((void*)D_8014686C, 0x1fu);
}
