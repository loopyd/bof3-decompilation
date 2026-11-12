#include "bof3/bof3.h"

/* Front-end record flag byte of the shared main-executable data segment: this
 * body clears its bit 0 with one read-modify-write, keeping the byte's other
 * seven bits.
 * @source 0x80145FF4 @kind data
 */
extern u8 D_80145FF4;
/* Second front-end record flag byte of the same segment, cleared here the same
 * way with one read-modify-write of bit 0.
 * @source 0x80146134 @kind data
 */
extern u8 D_80146134;
/* Shared front state byte: this body reads it and clears its bit 0 last, after
 * the primary state has been requested.
 * @source 0x80146871 @kind data
 */
extern u8 D_80146871;
/* Shared primary state byte of the front-end state tuple, stored with one `sb`.
 * @source 0x80146874 @kind data
 */
extern u8 D_80146874;
/* Shared secondary sub-state byte of the same tuple, stored with one `sb`.
 * @source 0x80146875 @kind data
 */
extern u8 D_80146875;

void func_8015C088(void);
void func_8019FA28(u16 selection_seed, u32 context_a, u32 context_b,
                   u8 context_kind);

/* @source 0x801F8850
 * @behavior Overlay front-end exit handler: it clears bit 0 of the two
 * front-end record flag bytes D_80145FF4 and D_80146134, seeds the front
 * selector context through func_8019FA28(0xC4, 0x298000, 0x168000, 0x80) -
 * selection seed 0xC4, the two 16.16 context words 0x298000 and 0x168000 and
 * context kind 0x80 - then runs the shared front-end startup helper
 * func_8015C088 and requests primary scene state 2 with secondary sub-state 0
 * by storing 2 in the shared primary state byte D_80146874 and 0 in the shared
 * secondary sub-state byte D_80146875. It finishes by clearing bit 0 of the
 * shared front state byte D_80146871. It takes no arguments and returns
 * nothing; its 0x18-byte frame only keeps $ra across the two calls. The first
 * record flag is read and written through a pointer local that the original
 * keeps in $a0 across both accesses, and that register then receives the first
 * argument 0xC4 in the func_8019FA28 jal delay slot while the other three
 * arguments are materialised above the $ra store; the second flag and the
 * three D_8014687x accesses keep the named-symbol form, and the D_80146871 load
 * is hoisted above the two state stores. Binding the first flag by its symbol
 * instead measured 28/35 instructions (80.00%) with only that register choice
 * and the argument materialisation moved, so the pointer local is the
 * evidenced representation. No in-image jal targets the address and no word of
 * this payload or of any other extracted payload holds it: 0x801F8850 appears
 * in neither form in the 396 payloads under out/binaries.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearRecordFlagBitsSeedFrontSelectorContextThenRequestPrimaryState2AndClearFrontStateScenarioScena1400_801F8850(
    void) {
  u8 *flags;

  flags = &D_80145FF4;
  *flags &= 0xFE;
  D_80146134 &= 0xFE;
  func_8019FA28(0xC4, 0x298000, 0x168000, 0x80);
  func_8015C088();
  D_80146874 = 2;
  D_80146875 = 0;
  D_80146871 &= 0xFE;
}
