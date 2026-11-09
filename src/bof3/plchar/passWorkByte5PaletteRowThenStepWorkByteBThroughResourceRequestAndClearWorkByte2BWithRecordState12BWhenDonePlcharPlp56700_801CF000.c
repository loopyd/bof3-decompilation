#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell. The original loads this cell ahead of
 * frame creation for the dispatch byte, keeps that pointer across the palette
 * step's own byte 5 read through a re-read, and re-reads the cell for every arm
 * and for the two clears, so the source keeps the named cell and evaluates it at
 * each use.
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

/* Readiness poll helper of the main executable; the sibling plchar lifts of this
 * body declare the same prototype. The original keeps only the returned byte as
 * the ready test, so the declared return type is that byte.
 */
extern u8 func_8014DAEC(void);

/* Shared main-executable resource request helper; the sibling plchar lifts of
 * this target declare the same prototype and pass a resource id.
 */
extern void func_8014D6B8(u32 flag);

/* Shared no-argument update helper of the main executable, outside this payload;
 * the sibling lifts of this body call it the same way.
 */
extern void func_8014D978(void);

/* Shared main-executable palette row helper, outside this payload. Its call
 * sites pass mode 0 and mode 1, and this step passes the 0x40-byte palette row
 * selected by work byte 5 with mode 0, so both parameter roles stay structural.
 */
extern void func_80166C48(u8* row, s32 mode);

/* Clone dry-run: this 0x130-byte span is byte-identical to the already exact
 * lifts of the same body at plp016 0x801CFB00, plp026 0x801CFF24, plp267
 * 0x801CF1FC and plp349 0x801CE9F4, compared over the original bytes of every
 * payload. The only differing words are the two intra-body jump targets at
 * 0x801CF0AC and 0x801CF0D0, each of which follows its own overlay's address, so
 * the statement order, the byte-0x0B dispatch, the pointer-cell evaluations and
 * the declaration style carry over with this function's own address and name.
 */
/* @source 0x801CF000
 * @behavior Overlay step reached through the code-pointer word 0x801CF000 at
 *           0x801D07C0, entry 13 of this overlay's twenty-four-word handler run
 *           at 0x801D078C that func_801CE404 indexes with work-record byte 2
 *           scaled by four before its indirect call, so the body runs while that
 *           byte reads 13; no jal in this payload names the span and the run word
 *           is its only in-image reference. When work byte 0x0B reads 1 the body
 *           first passes the 0x40-byte palette row at 0x80035600 selected by
 *           work byte 5 to the shared palette helper func_80166C48 with mode 0
 *           and writes 2 into work byte 0x0B. It then re-reads the pointer cell
 *           and dispatches on byte 0x0B again: a zero byte polls the shared
 *           readiness helper func_8014DAEC and reports done as soon as that
 *           helper answers nonzero, while a nonzero byte takes one of two arms
 *           and both of them then report done whenever the front-end phase byte
 *           0x80143BB0 reads zero. Byte 2 polls the same helper and, once it
 *           answers nonzero, passes work byte 8 to the shared resource request
 *           helper func_8014D6B8 and writes 3 into work byte 0x0B, and every
 *           other value runs the shared no-argument update helper func_8014D978.
 *           When the done flag is set the body clears work byte 0x2B and then
 *           byte 0x12B of the record published at 0x80146250, re-reading the
 *           pointer cell before the first store. It takes no arguments, returns
 *           nothing, and the 0x18-byte frame only keeps $ra at 0x14 and the done
 *           flag at 0x10 across the calls, the pointer cell is read ahead of
 *           frame creation, and every arm joins on one epilogue. The roles of
 *           work bytes 0x0B, 0x05, 0x08 and 0x2B and of the record's byte 0x12B
 *           are not established in this overlay, so the name describes the
 *           observed step and keeps those bytes address-anchored.
 * @status exact
 * @match 100.00
 * @residual none
 */
void passWorkByte5PaletteRowThenStepWorkByteBThroughResourceRequestAndClearWorkByte2BWithRecordState12BWhenDonePlcharPlp56700_801CF000(
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
