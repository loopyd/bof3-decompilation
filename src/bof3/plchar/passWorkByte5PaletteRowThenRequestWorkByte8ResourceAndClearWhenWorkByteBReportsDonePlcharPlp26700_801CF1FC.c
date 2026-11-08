#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell. The original loads this cell ahead of
 * frame creation, keeps that pointer for the work byte 5 read of the palette
 * step, and re-reads the cell for the dispatch byte and again for the clear, so
 * the source keeps the named cell and evaluates it at each use.
 * @source 0x1F800044 @kind data
 */
extern u8* D_1F800044;

/* Front-end record pointer cell of the shared main executable. The clear path
 * writes byte 0x12B of the record published here; the role of that byte is not
 * established in this overlay, so it stays address-anchored.
 * @source 0x80146250 @kind unknown
 */
extern u8* D_80146250;

/* Front-end phase byte of the shared main executable. This handler only tests it
 * for zero, so its role here stays address-anchored.
 * @source 0x80143BB0 @kind unknown
 */
extern u8 D_80143BB0;

/* Readiness poll helper of the main executable; the sibling lifts of this target
 * declare the same prototype. The body keeps only the returned byte as the ready
 * test, so the declared return type is that byte.
 */
extern u8 func_8014DAEC(void);

/* Shared main-executable resource request helper; the sibling lifts of this
 * target declare the same prototype and pass a resource id.
 */
extern void func_8014D6B8(u32 flag);

/* Shared no-argument update helper of the main executable, outside this payload;
 * the sibling lifts of this payload call it the same way.
 */
extern void func_8014D978(void);

/* Shared main-executable palette row helper, outside this payload. Its two call
 * sites in the bmagic overlay pass mode 0 and mode 1, and this step passes the
 * 0x40-byte palette row selected by work byte 5, so both parameter roles stay
 * structural.
 */
extern void func_80166C48(u8* row, s32 mode);

/* This body is the work byte 3 == 2 step of the byte-3 dispatcher of this
 * overlay. Its instruction stream is word-identical to eight other plchar
 * overlay spans once the two absolute jump immediates are masked, and two of
 * those already carry exact lifts: the plp016 body at 0x801CFB00 and the plp026
 * body at 0x801CFF24. Because only relocation immediates differ, those copies
 * are shape evidence rather than clone representatives, so this seed reuses their
 * proven statement order, operand forms and declaration style and states the
 * evidence this payload's own bytes carry.
 *
 * @source 0x801CF1FC
 * @behavior Work byte 3 == 2 step of the byte-3 dispatcher
 *           dispatchWorkByte3HandlerPlcharPlp26700_801CEE6C at 0x801CEE6C, which
 *           indexes the code-pointer run based at 0x801D09B0 with work record
 *           byte 3 scaled by four. The body address appears once in the payload
 *           bytes, as the run word 0x801CF1FC at 0x801D09B8, entry 2 of that run
 *           and index 12 of the thirty-word code-pointer run labelled T_801D0988
 *           that begins at 0x801D0988; no jal in this payload names the body, so
 *           the run is its only in-image reference. When work byte 0x0B reads 1
 *           it first passes the 0x40-byte palette row at 0x80035600 selected by
 *           work byte 5 to the shared palette helper func_80166C48 with mode 0
 *           and writes 2 into byte 0x0B. It then re-reads the pointer cell and
 *           dispatches on byte 0x0B again: zero polls the shared readiness helper
 *           func_8014DAEC and reports done as soon as that helper answers
 *           nonzero, while a nonzero byte takes one of two arms and both of them
 *           then report done whenever the front-end phase byte 0x80143BB0 reads
 *           zero: byte 2 polls the same helper and, once it answers nonzero,
 *           passes work byte 8 to the shared resource request helper func_8014D6B8
 *           and writes 3 into byte 0x0B, and every other value runs the shared
 *           no-argument update helper func_8014D978. When the done flag is set it
 *           clears work byte 0x2B and then byte 0x12B of the record published at
 *           0x80146250, re-reading both pointer cells for the stores. The
 *           0x18-byte frame only keeps $ra at 0x14 and the done flag at 0x10
 *           across the calls, and every arm joins on one epilogue. It takes no
 *           arguments and returns nothing. The roles of work bytes 0x0B, 0x05,
 *           0x2B and 8 are not established in this overlay, so the name describes
 *           the observed step and keeps those bytes address-anchored.
 * @status exact
 * @match 100.00
 * @residual none
 */
void passWorkByte5PaletteRowThenRequestWorkByte8ResourceAndClearWhenWorkByteBReportsDonePlcharPlp26700_801CF1FC(
    void) {
  u8 step;
  u8 done;

  step = D_1F800044[0xB];
  done = 0;
  if (step == 1) {
    func_80166C48((u8*)(0x80035600u + (D_1F800044[5] << 6)), 0);
    D_1F800044[0xB] = 2;
  }
  step = D_1F800044[0xB];
  if (step != 0) {
    if (step == 2) {
      if (func_8014DAEC()) {
        func_8014D6B8(D_1F800044[8]);
        D_1F800044[0xB] = 3;
      }
    } else {
      func_8014D978();
    }
    if (D_80143BB0 == 0) {
      done = 1;
    }
  } else {
    if (func_8014DAEC()) {
      done = 1;
    }
  }
  if (done) {
    D_1F800044[0x2B] = 0;
    D_80146250[0x12B] = 0;
  }
}
