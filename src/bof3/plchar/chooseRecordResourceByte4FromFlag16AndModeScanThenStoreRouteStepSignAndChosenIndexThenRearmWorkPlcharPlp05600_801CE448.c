#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell of the shared main executable. The
 * original re-reads the cell before every store of the body, because a store
 * through the pointer may alias the cell itself, so the source keeps the
 * named-symbol form and evaluates it again at every access.
 * @source 0x1F800044 @kind data
 */
extern u8* D_1F800044;

/* Flag-bank base of the shared main executable: the body hands this address to
 * the flag tester func_8015B5D4 together with the flag index 0x16, so it is the
 * base of the bank rather than a single byte and stays address-anchored.
 * @source 0x80144E90 @kind data
 */
extern u8 D_80144E90[];

/* Front-end flag byte of the shared main executable. The world and front lifts
 * of this image describe bit 0 as a pending scenario and bit 3 as an alternate
 * front mode; this body tests bit 6, whose role is not established here, so the
 * byte stays address-anchored.
 * @source 0x80143F02 @kind unknown
 */
extern u8 D_80143F02;

/* Shared flag tester of the shared main executable, outside this payload: it
 * takes the bank base and the flag index and answers nonzero when that flag is
 * set. This body asks it for index 0x16 of the bank above.
 */
s32 func_8015B5D4(u32 base, s32 flag);

/* Mode scanner of the shared main executable, outside this payload: it takes a
 * zero-extended selector and answers one byte. This body asks for selector 0,
 * the form the shop front-end lifts of this image also use, and compares the
 * answer with 1.
 */
u8 func_801BDB7C(u8 arg0);

/* Route step-sign helper of the shared main executable, outside this payload:
 * it classifies a requested route index against the work route byte 0x08 within
 * a wrapped eight-entry window and answers -1 when the bound stands one to four
 * entries above the request and 1 otherwise. It takes one zero-extended byte
 * argument, so every call here passes a small immediate.
 */
s32 func_801B2D00(u8 index);

/* @source 0x801CE448
 * @behavior Work-state handler of the front-end work object read through the
 *           scratchpad pointer cell 0x1F800044. The body address appears
 *           nowhere in this payload: no jal immediate targets it and no word of
 *           the payload holds it, so it is reached from outside the payload,
 *           takes no arguments and answers no value. Byte 0x04 is the record
 *           resource id of that work object, byte 0x08 the current route index
 *           of the eight-entry route window, byte 0x03 the chosen candidate
 *           index, byte 0x0B the step sign, byte 0x09 the countdown and byte
 *           0x02 the step counter. The body first asks the shared flag tester
 *           func_8015B5D4 for flag 0x16 of the bank based at D_80144E90. While
 *           that flag reads clear it stores 0x41 into the resource byte; when it
 *           reads set it scans with func_801BDB7C(0) and, if that answer is
 *           exactly 1 while bit 0x40 of the front-end flag byte D_80143F02
 *           reads clear, it stores 0x50 there instead, and 0x40 otherwise. It
 *           then dispatches on the resource byte it just wrote: 0x41 stores the
 *           step sign func_801B2D00(5) in byte 0x0B and the chosen index 5 in
 *           byte 0x03; 0x40 does the same for route 3; 0x50 computes the
 *           distance of route byte 0x08 from 5 and from 3, five first, each as
 *           the absolute value of the difference, and when the distance to 3 is
 *           the smaller one stores the sign towards 3 and index 3, otherwise
 *           the sign towards 5, index 5, and moves the resource byte itself to
 *           0x52; every other resource value leaves the dispatch without a
 *           write. All paths join on the tail that sets countdown byte 0x09 to 2
 *           and increments the step counter byte 0x02, which re-arms the work
 *           object for the selected route. The 0x18-byte frame only keeps the
 *           return address at 0x10 across the three calls, and the pointer cell
 *           is re-read before every store because a store through the pointer
 *           may alias the cell itself. The 0x41 store of the entry block shares
 *           one store site with the 0x40 arm, whose constant 0x40 the original
 *           materialises in the delay slot of the jump to that site and whose
 *           0x41 the entry test materialises in its own delay slot; the
 *           resource test loads the cell once and the 0x50 arm reads route byte
 *           0x08 from that same load.
 *
 * Shape note: one cmp of the extracted 0x1E0-byte spans proved this body
 * byte-identical to the same-address span of the plp012, plp015, plp016,
 * plp025, plp026 and plp034 payloads (sha256 d8a737595822f11c3504c0faa1adf06dd911bd85d0e8679b5ede4213a7f7c39c),
 * and every one of those rows still stands on asm, so the clone lever had no
 * lifted source to apply and was declined. The same-target exact lift of the
 * body at 0x801CF170 supplied the statement order, absolute-value pair, operand
 * forms and declaration style of the 0x50 arm and of the re-arm tail, which it
 * carries word for word; the resource constants, route indices, the 0x52 store
 * and the entry block come from this span's own bytes.
 *
 * Rung evidence: the switch arm order is the whole residual. Writing the three
 * arms in the source order 0x40, 0x41, 0x50 measured 91/120 instructions with
 * the first difference at instruction 37, the target of the 0x41 test, because
 * the compiler emits the arm bodies in its own order and only one source order
 * lands them where the original has them; 0x41, 0x50, 0x40 measured 89/120 and
 * 0x50, 0x41, 0x40 measured 91/120, all with that same first difference. The
 * order 0x50, 0x40, 0x41 written here measured 120/120 with no instruction
 * differing, so no other change was needed.
 * @status exact
 * @match 100.00
 * @residual none
 */
void chooseRecordResourceByte4FromFlag16AndModeScanThenStoreRouteStepSignAndChosenIndexThenRearmWorkPlcharPlp05600_801CE448(
    void) {
  s32 fromFive;
  s32 fromThree;

  if (func_8015B5D4((u32)D_80144E90, 0x16) != 0) {
    if (func_801BDB7C(0) == 1 && (D_80143F02 & 0x40) == 0) {
      D_1F800044[4] = 0x50;
    } else {
      D_1F800044[4] = 0x40;
    }
  } else {
    D_1F800044[4] = 0x41;
  }
  switch (D_1F800044[4]) {
    case 0x50:
      fromFive = D_1F800044[8] - 5;
      if (fromFive < 0) {
        fromFive = -fromFive;
      }
      fromThree = D_1F800044[8] - 3;
      if (fromThree < 0) {
        fromThree = -fromThree;
      }
      if (fromThree < fromFive) {
        D_1F800044[0xB] = func_801B2D00(3);
        D_1F800044[3] = 3;
      } else {
        D_1F800044[0xB] = func_801B2D00(5);
        D_1F800044[3] = 5;
        D_1F800044[4] = 0x52;
      }
      break;
    case 0x40:
      D_1F800044[0xB] = func_801B2D00(3);
      D_1F800044[3] = 3;
      break;
    case 0x41:
      D_1F800044[0xB] = func_801B2D00(5);
      D_1F800044[3] = 5;
      break;
    default:
      break;
  }
  D_1F800044[9] = 2;
  D_1F800044[2] = D_1F800044[2] + 1;
}
