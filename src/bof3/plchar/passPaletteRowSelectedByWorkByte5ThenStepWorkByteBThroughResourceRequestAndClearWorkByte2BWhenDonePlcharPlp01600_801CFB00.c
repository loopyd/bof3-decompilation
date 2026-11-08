#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell of the shared main executable. This body
 * reads it once for the byte-1 test and the row selector, re-reads it for the
 * step dispatch because the row helper call may republish the cell, and
 * re-reads it again before each store for the same reason.
 * @source 0x1F800044 @kind data
 */
extern u8* D_1F800044;

/* Front-end record pointer cell of the shared main executable. The body clears
 * byte 0x12B of the record it publishes when the step reports done.
 * @source 0x80146250 @kind unknown
 */
extern u8* D_80146250;

/* Local state byte of the shared main executable: the front-end handlers that
 * own it treat zero as free, and a busy value suppresses the clear.
 * @source 0x80143BB0 @kind unknown
 */
extern u8 D_80143BB0;

/* Readiness poll helper of the shared EXE image, outside this payload; the
 * battle, bmagic and sibling plchar lifts declare the same prototype. This body
 * keeps only the returned byte as the ready test, so the declared return type is
 * that byte.
 */
u8 func_8014DAEC(void);

/* Shared main-executable resource request helper, outside this payload; the
 * sibling plchar lifts declare the same prototype and pass a resource id.
 */
void func_8014D6B8(u32 flag);

/* Shared no-argument update helper of the shared EXE image, outside this
 * payload; the sibling lifts of this payload call it the same way.
 */
void func_8014D978(void);

/* Row helper of the shared EXE image, outside this payload; only this body of
 * the payload calls it, and it receives the 0x80035600 palette block base plus
 * (work byte 5 << 6) as its first argument and zero as its second. Its own role
 * is not established here, so the prototype carries the observed call shape and
 * the name keeps the address.
 */
void func_80166C48(u8* row, u32 flag);

/* @source 0x801CFB00
 * @behavior Work byte 3 == 2 step of the front-end work object read through the
 *           scratchpad pointer cell 0x1F800044. Its only in-image reference is
 *           the word 0x801CFB00 at 0x801D0678, entry 2 of the code-pointer run
 *           based at 0x801D0670 that the byte-3 dispatcher func_801CF770 indexes
 *           with work byte 0x03 scaled by four; no jal in this payload names it,
 *           so it is reached through the run alone. The body drives the work
 *           object's unsigned byte 0x0B, a sub-step counter of this byte-3 arm.
 *           When byte 0x0B reads one the body passes the palette block row at
 *           0x80035600 + (work byte 0x05 << 6) to the shared row helper
 *           func_80166C48 with zero as its second argument, then re-reads the
 *           pointer cell and writes 2 into byte 0x0B so the dispatch below takes
 *           its ready arm in the same invocation. The dispatch re-reads the cell
 *           for the new value: byte 0x0B == 2 polls the shared readiness helper
 *           func_8014DAEC and, once that helper answers nonzero, passes work byte
 *           0x08 to the shared resource request helper func_8014D6B8 and writes
 *           3 into byte 0x0B; byte 0x0B == 0 only polls the same readiness
 *           helper and reports done as soon as it answers nonzero; every other
 *           value runs the shared no-argument update helper func_8014D978. The
 *           local state byte 0x80143BB0 then decides the done flag for the
 *           byte-3 arm: a zero there reports done, a busy value suppresses the
 *           clear, and the byte-0x0B == 0 arm never consults it. When the flag is
 *           set the handler clears work byte 0x2B and then clears the state byte
 *           0x12B of the front-end record published at 0x80146250, re-reading
 *           both pointer cells for the stores; both arms join on one epilogue.
 *           It takes no arguments, returns nothing, and the 0x18-byte frame only
 *           keeps $ra at 0x14 and the flag register at 0x10 across the calls. The
 *           source binds the first read of byte 0x0B to a local and zeroes the
 *           done flag before the byte-0x0B == 1 test instead of after it; that
 *           ordering is what reproduces the original's prologue save order and
 *           the zeroed flag in the first branch's delay slot, and the dispatch
 *           then re-reads byte 0x0B into the same local. The
 *           span is 0x130 bytes at payload offset 0x1700. The roles of work byte
 *           0x0B, of work byte 0x2B, of the row helper and of the requested
 *           resource are not established here, so the name describes the
 *           observed sub-steps and keeps the address.
 * @status exact
 * @match 100.00
 * @residual none
 */
void passPaletteRowSelectedByWorkByte5ThenStepWorkByteBThroughResourceRequestAndClearWorkByte2BWhenDonePlcharPlp01600_801CFB00(
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
