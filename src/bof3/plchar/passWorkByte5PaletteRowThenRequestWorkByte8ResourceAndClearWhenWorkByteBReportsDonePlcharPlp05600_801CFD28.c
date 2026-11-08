#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell of the shared main executable. This body
 * reads it once for the byte-0x0B test and the row selector, re-reads it for the
 * sub-step dispatch because the row helper call may republish the cell, and
 * re-reads it again before each store for the same reason, so the source keeps
 * the named cell and evaluates it at each use.
 * @source 0x1F800044 @kind data
 */
extern u8* D_1F800044;

/* Front-end record pointer cell of the shared main executable. The done path
 * clears byte 0x12B of the record published here; the role of that byte is not
 * established in this overlay, so it stays address-anchored.
 * @source 0x80146250 @kind unknown
 */
extern u8* D_80146250;

/* Front-end phase byte of the shared main executable. Other lifts store the
 * values 0 and 5 into it and treat 2 as busy; this handler only reads it for
 * zero, and only on the two nonzero arms of the sub-step dispatch, so its role
 * here stays address-anchored.
 * @source 0x80143BB0 @kind unknown
 */
extern u8 D_80143BB0;

/* Readiness poll helper of the shared EXE image, outside this payload; the
 * sibling lifts of this target declare the same prototype. The body keeps only
 * the returned byte as the ready test, so the declared return type is that byte.
 */
extern u8 func_8014DAEC(void);

/* Shared main-executable resource request helper; the sibling lifts of this
 * target declare the same prototype and pass a resource id.
 */
extern void func_8014D6B8(u32 flag);

/* Shared no-argument update helper of the main executable, outside this
 * payload; the sibling lifts of this payload call it the same way.
 */
extern void func_8014D978(void);

/* Row helper of the shared EXE image, outside this payload; only this body of
 * the payload calls it, and it receives the 0x80035600 palette block base plus
 * (work byte 5 << 6) as its first argument and zero as its second. Its own role
 * is not established here, so the prototype carries the observed call shape.
 * This target's generated undefined-funcs file still carries the row for
 * 0x80166C48 after the asm-to-C flip, so no authored map row is added for it.
 */
extern void func_80166C48(u8* row, s32 mode);

/* @source 0x801CFD28
 * @behavior Work-byte-2 handler of this overlay, entry 19 of the code-pointer
 *           run based at 0x801D0858: the run's twentieth word is 0x801CFD28 at
 *           0x801D08A4, and the overlay's work-byte-2 dispatcher func_801CE404
 *           indexes that run with the unsigned byte 2 of the work object read
 *           through the scratchpad pointer cell 0x1F800044. No jal in this
 *           payload names this function and no other word in the payload holds
 *           its address, so the run, which sits in a `textbin` row, is its only
 *           in-image reference and the loader that steps the run is its caller.
 *           The body drives work-object unsigned byte 0x0B, a sub-step counter:
 *           when it reads 1 the body passes the 0x40-byte row of the
 *           main-executable palette block at 0x80035600 selected by work byte
 *           0x05 to the shared row helper func_80166C48 with mode 0, then
 *           re-reads the pointer cell and writes 2 into byte 0x0B. It then
 *           re-reads byte 0x0B and dispatches again in the same invocation: zero
 *           polls the shared readiness helper func_8014DAEC and reports done as
 *           soon as that helper answers nonzero, the value 2 polls the same
 *           helper and, once it answers nonzero, passes work byte 0x08 to the
 *           shared resource request helper func_8014D6B8 and writes 3 into byte
 *           0x0B, and every other value runs the shared no-argument update
 *           helper func_8014D978. The front-end phase byte 0x80143BB0 then
 *           decides the done flag for both nonzero arms of that dispatch: a zero
 *           there reports done, a busy value suppresses the clear, and the
 *           byte-0x0B == 0 arm never consults it. When the flag is set the body
 *           clears work byte 0x2B and then byte 0x12B of the record published at
 *           0x80146250, re-reading both pointer cells for the stores; every arm
 *           joins on one epilogue. It takes no arguments and returns nothing,
 *           the dispatcher discards $v0, and the 0x18-byte frame only keeps the
 *           done flag in $s0 at 0x10 and $ra at 0x14 across the calls. The roles
 *           of work bytes 0x0B, 0x05, 0x2B and 0x08, of the 0x80035600 block, of
 *           the row helper and of the requested resource are not established in
 *           this overlay, so the name describes the observed step and keeps
 *           those bytes address-anchored. The span is 0x130 bytes at payload
 *           offset 0x1928. It has no byte-identical twin in the extracted
 *           payload corpus: a scan of all 396 payload and EXE images found nine
 *           occurrences of its first 0xAC bytes, this payload's own span plus
 *           eight relocation-different siblings, and each sibling equals this
 *           body in every word except the two `j` words at span bytes 0xAC and
 *           0xD0, whose immediates encode each span's own local labels and each
 *           shift by exactly the delta between the two spans (0x8A words for the
 *           PLP016 sibling at 0x228), so those twins are shape evidence rather
 *           than clone candidates. The shape was taken from the exact lifts of
 *           0x801CFB00 in PLP016, 0x801CFF24 in PLP026 and 0x801CF1FC in PLP267,
 *           whose bodies state the same behavior, and the name follows the two
 *           of them that share this spelling.
 * @status exact
 * @match 100.00
 * @residual none
 */
void passWorkByte5PaletteRowThenRequestWorkByte8ResourceAndClearWhenWorkByteBReportsDonePlcharPlp05600_801CFD28(
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
