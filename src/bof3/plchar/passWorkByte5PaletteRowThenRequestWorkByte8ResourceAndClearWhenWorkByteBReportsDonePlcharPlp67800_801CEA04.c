#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell of the shared main executable. The
 * original loads this cell once ahead of frame creation, keeps that pointer for
 * the work byte 5 read of the palette step, and re-reads the cell for the
 * dispatch byte, for the resource-request argument, and again for the two
 * clears, so the source keeps the named cell and evaluates it at each use the
 * original re-loads.
 * @source 0x1F800044 @kind data
 */
extern u8* D_1F800044;

/* Front-end record pointer cell of the shared main executable. The clear path
 * writes byte 0x12B of the record published here; the role of that byte is not
 * established in this overlay, so it stays address-anchored.
 * @source 0x80146250 @kind unknown
 */
extern u8* D_80146250;

/* Front-end phase byte of the shared main executable. Both nonzero arms of the
 * byte 0x0B dispatch test it for zero before they can report done, so the role
 * stays address-anchored.
 * @source 0x80143BB0 @kind unknown
 */
extern u8 D_80143BB0;

/* Readiness poll helper of the main executable. Both polling arms of this body
 * keep only the returned byte as the ready test, so the declared return type is
 * that byte. This target generated undefined_funcs_auto.txt binds the address,
 * and the other lifts of this target declare the same prototype.
 * @source 0x8014DAEC
 */
extern u8 func_8014DAEC(void);

/* Shared main-executable resource request helper. The body hands it the work
 * object unsigned byte 0x08 as the requested id and discards any answer. This
 * target generated undefined_funcs_auto.txt binds the address, and the exact
 * lift at 0x801CEB34 of this target declares the same prototype.
 * @source 0x8014D6B8
 */
extern void func_8014D6B8(u32 flag);

/* Shared no-argument update helper of the main executable, outside this
 * payload; the third arm of this body calls it and reads no result. This target
 * generated undefined_funcs_auto.txt binds the address.
 */
extern void func_8014D978(void);

/* Shared main-executable palette helper, outside this payload. The bmagic
 * overlays pass mode 0 and mode 1 at their call sites, and this step passes the
 * 0x40-byte palette row selected by work byte 5 with mode 0, so both parameter
 * roles stay structural. This target generated undefined_funcs_auto.txt binds
 * the address.
 */
extern void func_80166C48(u8* row, s32 mode);

/* Shape note: this 0x130-byte span is relocation-different from seven already
 * exact lifts of the same body, at plp467 0x801CEC20, plp016 0x801CFB00,
 * plp026 0x801CFF24, plp267 0x801CF1FC, plp349 0x801CE9F4, plp567 0x801CF000
 * and plp056 0x801CFD28, and from the still unlifted plp034 span at 0x801CF71C.
 * A little-endian scan of every payload under out/binaries with the five `jal`
 * words and the two interior `j` words masked matched exactly those eight spans
 * and this one, and each of the eight differs from this span in exactly two
 * words, the `j` words at span offsets 0xAC and 0xD0 that name this body's own
 * interior labels and therefore encode the span base address. The difference of
 * each of those two words equals the base-address delta between the spans, so
 * this is relocation-different shape evidence and not a clone: no payload holds
 * these 0x130 bytes verbatim. The statement order, the single dispatch-byte
 * read, the reload-per-access pointer-cell form and the declaration style carry
 * over from the plp026 lift, whose own gate measured exact.
 */
/* @source 0x801CEA04
 * @behavior Work byte 2 == 7 step of the scratch work object read through the
 *           pointer cell 0x1F800044. The body address appears once in the
 *           payload, as the word 0x801CEA04 at 0x801D0580, entry 7 of the
 *           thirty-word code-pointer run based at 0x801D0564, which this
 *           overlay places as its textbin row at that base, and the byte-2
 *           dispatcher func_801CE404 indexes that run with work-record byte 2
 *           scaled by four before its indirect call, so the body runs while
 *           that byte reads 7 and no jal or j word of the payload names it. It
 *           takes no arguments and any result is discarded. When work byte 0x0B
 *           reads 1 the body first passes the 0x40-byte palette row at
 *           0x80035600 selected by work byte 5 to the shared palette helper
 *           func_80166C48 with mode 0 and writes 2 into byte 0x0B, so a first
 *           call opens with that palette step and then dispatches again on the
 *           byte it just wrote. The dispatch on byte 0x0B keeps one done flag
 *           across the calls, which is why the flag survives them in a
 *           callee-saved register: zero polls the shared readiness helper
 *           func_8014DAEC and reports done as soon as that helper answers
 *           nonzero, while a nonzero byte takes one of two arms and both of
 *           them then report done whenever the front-end phase byte 0x80143BB0
 *           reads zero. Byte 2 polls the same helper and, once it answers
 *           nonzero, passes work byte 8 to the shared resource request helper
 *           func_8014D6B8 and writes 3 into byte 0x0B; every other value runs
 *           the shared no-argument update helper func_8014D978. When the done
 *           flag is set the body clears work byte 0x2B and then byte 0x12B of
 *           the record published at 0x80146250, re-reading both pointer cells
 *           for the two stores. It returns nothing, the 0x18-byte frame only
 *           keeps $ra at 0x14 and the done flag at 0x10 across the calls, and
 *           every arm joins on one epilogue. The roles of work bytes 0x0B,
 *           0x05, 0x2B and 8, of the requested resource and of the cleared
 *           record byte are not established here, so the name describes the
 *           observed step and keeps those bytes address-anchored.
 * @status exact
 * @match 100.00
 * @residual none
 */
void passWorkByte5PaletteRowThenRequestWorkByte8ResourceAndClearWhenWorkByteBReportsDonePlcharPlp67800_801CEA04(
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
