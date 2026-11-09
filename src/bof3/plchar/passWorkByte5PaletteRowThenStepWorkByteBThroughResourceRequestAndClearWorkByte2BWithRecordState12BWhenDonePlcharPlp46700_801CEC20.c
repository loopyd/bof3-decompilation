#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell of the shared main executable. The body
 * reads it ahead of frame creation for the byte it dispatches on, reads work byte
 * 5 through that first pointer for the palette row, and re-reads the cell before
 * the second dispatch, before the resource request's argument read, before each
 * of the two byte-0x0B stores and before the first clear, because a call may
 * change the record the cell points at. The source keeps the named cell and
 * evaluates it again at each access the original reloads.
 * @source 0x1F800044 @kind data
 */
extern u8* D_1F800044;

/* Front-end record pointer cell of the shared main executable. The body clears
 * byte 0x12B of the record this cell publishes when the step reports done; the
 * lifted siblings of this overlay read the same cell and its role here is not
 * established, so it stays address-anchored.
 * @source 0x80146250 @kind unknown
 */
extern u8* D_80146250;

/* Front-end phase byte of the shared main executable. The body reports done
 * whenever this byte reads zero, and a nonzero value therefore suppresses both
 * clears. The sibling lifts of the plchar overlays read the same byte with
 * differing roles, so it stays address-anchored here.
 * @source 0x80143BB0 @kind unknown
 */
extern u8 D_80143BB0;

/* Shared readiness helper of the shared main executable, outside this payload:
 * it answers an unsigned byte and every polling arm of this body only tests that
 * byte for zero. Splat binds its address through this target's generated
 * undefined_funcs_auto.txt, and the lifted siblings of this overlay call it the
 * same way.
 * @source 0x8014DAEC
 */
extern u8 func_8014DAEC(void);

/* Shared main-executable resource request helper. The body hands it the work
 * object's unsigned byte 0x08 as the requested id and discards any answer; the
 * exact sibling lifts of this body in other plchar overlays declare the same
 * prototype.
 * @source 0x8014D6B8
 */
extern void func_8014D6B8(u32 flag);

/* Shared no-argument update helper of the main executable, outside this payload;
 * the last arm of this body calls it and reads no result. The lifted siblings of
 * this overlay call it the same way.
 * @source 0x8014D978
 */
extern void func_8014D978(void);

/* Shared main-executable palette row helper, outside this payload. Its call
 * sites pass mode 0 and mode 1, and this step passes the 0x40-byte palette row
 * selected by work byte 5 with mode 0, so both parameter roles stay structural.
 * @source 0x80166C48
 */
extern void func_80166C48(u8* row, s32 mode);

/* Shape seed: the 0x130-byte body is instruction-for-instruction identical to the
 * exact lifted bodies of six other plchar overlays - plp016 0x801CFB00, plp026
 * 0x801CFF24, plp056 0x801CFD28, plp267 0x801CF1FC, plp349 0x801CE9F4 and
 * plp567 0x801CF000 - compared over the original payload bytes. Each of those
 * differs from this span in exactly two words, the `j` words at 0x801CECCC and
 * 0x801CECF0 that name the body's own interior labels and therefore encode the
 * function's base address, so this is relocation-different shape evidence and
 * not a clone; the statement order, the byte-0x0B dispatch, the reload-per-access
 * pointer-cell form and the declaration style carry over with this function's own
 * address and name.
 */
/* @source 0x801CEC20
 * @behavior Work byte 3 == 2 step of this overlay, the word 0x801CEC20 at
 *           0x801D03D4: entry 10 of the twenty-eight-word code-pointer run based
 *           at 0x801D03AC and index 2 of the eight-word run based at 0x801D03CC
 *           that this target byte-3 dispatcher
 *           dispatchWorkByte3Table03ccPlcharPlp46700_801CE890 enters by scaling
 *           work byte 3 by four before its indirect call, so no jal in this
 *           payload names the span and those two run words are its only in-image
 *           references. When work byte 0x0B reads 1 the body first passes the
 *           0x40-byte palette row at 0x80035600 selected by work byte 5 to the
 *           shared palette helper func_80166C48 with mode 0 and writes 2 into work
 *           byte 0x0B. It then re-reads the pointer cell and dispatches on byte
 *           0x0B again: a zero byte polls the shared readiness helper
 *           func_8014DAEC and reports done as soon as that helper answers nonzero,
 *           while a nonzero byte takes one of two arms and both of them then
 *           report done whenever the front-end phase byte 0x80143BB0 reads zero.
 *           Byte 2 polls the same helper and, once it answers nonzero, passes
 *           work byte 8 to the shared resource request helper func_8014D6B8 and
 *           writes 3 into work byte 0x0B, and every other value runs the shared
 *           no-argument update helper func_8014D978. When the done flag is set
 *           the body clears work byte 0x2B and then byte 0x12B of the record
 *           published at 0x80146250, re-reading the pointer cell before the first
 *           store. It takes no arguments and returns nothing; the 0x18-byte frame
 *           only keeps $ra at 0x14 and the done flag at 0x10 across the calls, the
 *           pointer cell is read ahead of frame creation and is not re-read for
 *           the palette step's byte 5, and every arm joins on one epilogue through
 *           the shared masked-flag test at 0x801CED10. The two nonzero arms
 *           share the phase-byte test at 0x801CECDC. The roles of work bytes
 *           0x0B, 0x05, 0x08 and 0x2B and of the record's byte 0x12B are not
 *           established here, so the name describes the observed step and keeps
 *           those bytes address-anchored.
 * @status exact
 * @match 100.00
 * @residual none
 */
void passWorkByte5PaletteRowThenStepWorkByteBThroughResourceRequestAndClearWorkByte2BWithRecordState12BWhenDonePlcharPlp46700_801CEC20(
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
