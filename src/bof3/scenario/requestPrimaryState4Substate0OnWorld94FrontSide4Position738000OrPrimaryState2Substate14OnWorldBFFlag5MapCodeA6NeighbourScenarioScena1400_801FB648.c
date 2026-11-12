#include "bof3/bof3.h"

/* Shared world id halfword of the main-executable data segment: this body reads
 * it twice, once for the 0x94 gate and once for the 0xBF gate.
 * @source 0x80143F00 @kind unknown
 */
extern u16 D_80143F00;
/* Front-side byte of the same segment, read once under the 0x94 world gate.
 * @source 0x80143F03 @kind unknown
 */
extern u8 D_80143F03;
/* Shared scenario flag word, handed to the main-executable flag tester
 * func_8015B5D4 with ids 0x10 and 5.
 * @source 0x8014686C @kind data
 */
extern u32 D_8014686C;
/* Shared primary state byte of the front-end state tuple, stored with one `sb`
 * on each success arm.
 * @source 0x80146874 @kind data
 */
extern u8 D_80146874;
/* Shared secondary sub-state byte of the same tuple, stored with one `sb` on
 * each success arm.
 * @source 0x80146875 @kind data
 */
extern u8 D_80146875;

s32 func_8015B5D4(u32 flags, s32 id);
void func_8015C088(void);
u8 func_80166CB0(s16 col, s16 row);

/* @source 0x801FB648
 * @behavior Overlay front-end gate that chooses between two primary-state
 * requests. The first arm needs the shared world id halfword D_80143F00 to read
 * 0x94, the front-side byte D_80143F03 to read 4, func_8015B5D4 to report flag
 * 0x10 clear in the shared flag word D_8014686C, the first argument to equal the
 * packed 16.16 position word 0x738000 and the arithmetic high half of the second
 * argument to lie in 0x36..0x37; it then runs the shared front-end startup helper
 * func_8015C088, requests primary scene state 4 with secondary sub-state 0 by
 * storing 4 in the shared primary state byte D_80146874 and 0 in the shared
 * secondary sub-state byte D_80146875, and returns 1. Failing any test it falls
 * through to the second arm, which re-reads D_80143F00, needs it to read 0xBF and
 * needs func_8015B5D4 to report flag 5 set; it then probes the map code under the
 * projected position through func_80166CB0, counting the answer 0xA6 at that cell
 * and, only while the matching low 16 bits of the argument are nonzero, at the
 * cell one further along the first coordinate, at the cell one further along the
 * second coordinate and at the cell one further along both, and on a nonzero count
 * runs func_8015C088 and requests primary state 2 with secondary sub-state 0x14
 * before returning 1. Every other path returns 0.
 *
 * Representation notes. The whole 0x204-byte span is one function: a 0x30-byte
 * frame that saves $s0 through $s6 plus $ra, one prologue and one `jr $ra`, and
 * no `jal` immediate or pointer word of this payload names the address, so the
 * clone lever is ineligible for want of any byte-identical copy: a scan of the
 * 6344 payloads under out/extracted found the span only in this target's own
 * SCENA14 slot 0 payload. The measured residual is allocation. The retained body
 * leaves the four probes taking the $s32 position locals at every site and keeps
 * the map code in a $u8 temporary, which measures 90/129 instructions and 508
 * bytes against the original 516. Spelling the probes with the position halves
 * written out at the last two sites instead rematerialises the two `sra`s the way
 * the original does but measures 86/129 (508 bytes), because the original also
 * loads the 0xA6 comparison constant into $s6 ahead of the second probe's call
 * and keeps that eighth saved register; no spelling tried here makes this
 * compiler materialise that constant across that call, so the remaining
 * difference is one register class plus the numbering shift it drags through the
 * body. Yoda spellings of the probe comparisons, a single-counter body without
 * the $u8 centre value, $s32 counters and locally named comparison constants all
 * measured the same 90/129 or 86/129 and are not retained.
 * @status partial
 * @match 69.77
 * @residual The original loads the 0xA6 comparison constant into $s6 before the
 * second probe's func_80166CB0 call and compares against it after the call; this
 * body materialises the same constant per site instead, so it saves seven
 * registers where the original saves eight and the saved-register numbering
 * shifts by one. Control flow, statement order, literals and every call site
 * already agree.
 */
s32 requestPrimaryState4Substate0OnWorld94FrontSide4Position738000OrPrimaryState2Substate14OnWorldBFFlag5MapCodeA6NeighbourScenarioScena1400_801FB648(
    s32 arg0, s32 arg1) {
  s32 col;
  s32 row;
  u8 code;
  u8 centre;
  u8 hits;

  if (D_80143F00 == 0x94 && D_80143F03 == 4 &&
      func_8015B5D4(D_8014686C, 0x10) == 0 && arg0 == 0x738000 &&
      (u32)((arg1 >> 16) - 0x36) < 2u) {
    func_8015C088();
    D_80146874 = 4;
    D_80146875 = 0;
    return 1;
  }
  if (D_80143F00 != 0xBF) {
    return 0;
  }
  if (func_8015B5D4(D_8014686C, 5) == 0) {
    return 0;
  }
  col = arg0 >> 16;
  row = arg1 >> 16;
  code = func_80166CB0(col, row);
  centre = (code == 0xA6);
  hits = centre;
  if ((arg0 & 0xFFFF) != 0) {
    code = func_80166CB0(col + 1, row);
    if (code == 0xA6) {
      hits = centre + 1;
    }
  }
  if ((arg1 & 0xFFFF) != 0) {
    code = func_80166CB0(col, row + 1);
    if (code == 0xA6) {
      hits += 1;
    }
  }
  if ((arg0 & 0xFFFF) != 0 && (arg1 & 0xFFFF) != 0) {
    code = func_80166CB0(col + 1, row + 1);
    if (code == 0xA6) {
      hits += 1;
    }
  }
  if (hits == 0) {
    return 0;
  }
  func_8015C088();
  D_80146874 = 2;
  D_80146875 = 0x14;
  return 1;
}
