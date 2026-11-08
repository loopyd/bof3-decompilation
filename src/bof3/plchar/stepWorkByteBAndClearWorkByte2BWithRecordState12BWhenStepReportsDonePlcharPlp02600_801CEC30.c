#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell. The original loads this cell ahead of
 * frame creation for the dispatch byte, then loads it again in the arm that
 * writes the step byte and once more for the clear, so the source keeps the
 * named cell and evaluates it at each use.
 * @source 0x1F800044 @kind data
 */
extern u8* D_1F800044;

/* Front-end record pointer cell of the shared main executable. The clear path
 * writes byte 0x12B of the record published here; the role of that byte is not
 * established in this overlay, so it stays address-anchored.
 * @source 0x80146250 @kind unknown
 */
extern u8* D_80146250;

/* Front-end phase byte of the shared main executable. Other lifts store the
 * values 0 and 5 into it and treat 2 as busy; this handler only reads it for
 * zero, so its role here stays address-anchored.
 * @source 0x80143BB0 @kind unknown
 */
extern u8 D_80143BB0;

/* Readiness poll helper of the main executable; the sibling lifts of this
 * target declare the same prototype. The body keeps only the returned byte as
 * the ready test, so the declared return type is that byte.
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

/* @source 0x801CEC30
 * @behavior Work-step handler of the scratch work object read through the
 *           pointer cell 0x1F800044. Its only in-image reference is the word
 *           0x801CEC30 at 0x801D0A68, entry 5 of this payload's thirty-word
 *           code-pointer run based at 0x801D0A54, which the dispatcher
 *           dispatchStateByte2Table0a54PlcharPlp02600_801CE404 indexes with work
 *           record byte 2, so the overlay loader that steps that run is what
 *           calls it. The body dispatches on the unsigned byte 0x0B of the work
 *           object and keeps one done flag across the dispatch, which is why the
 *           flag has to survive the calls in a callee-saved register. When byte
 *           0x0B is zero the body only polls the shared readiness helper and
 *           reports done as soon as that helper answers nonzero. When byte 0x0B
 *           is one it polls the same helper and, once the helper answers
 *           nonzero, passes the work-object unsigned byte 0x08 to the shared
 *           resource request helper and writes 2 into byte 0x0B so the next
 *           dispatch takes the third arm; this arm never reports done. For every
 *           other value of byte 0x0B the body runs the shared no-argument update
 *           helper and reports done whenever the front-end phase byte 0x80143BB0
 *           reads zero, so a busy phase suppresses the clear. When the flag is
 *           set the handler clears the work-object byte 0x2B and then clears byte
 *           0x12B of the record published at 0x80146250, re-reading both pointer
 *           cells for the stores. It takes no arguments and returns nothing; the
 *           dispatcher discards the result and the arm that advances byte 0x0B
 *           exits with the flag still clear. The 0x18-byte frame only keeps $ra
 *           at 0x14 and the flag register at 0x10 across the calls, and every
 *           arm joins on one epilogue. The roles of work byte 0x0B, of work byte
 *           0x2B and of the requested resource are not established here, so the
 *           name describes the observed step and keeps the addresses. Its
 *           236-byte body is byte-identical to the exact lift of 0x801CF35C in
 *           PLP27A apart from the two `j` immediates that encode each function's
 *           own local label (cmp of the two payload spans: bytes 104 and 140
 *           differ, all other 232 bytes equal), so that twin is shape evidence
 *           and not a clone, and the shape was taken from it.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stepWorkByteBAndClearWorkByte2BWithRecordState12BWhenStepReportsDonePlcharPlp02600_801CEC30(
    void) {
  u8 step;
  u8 done;

  step = D_1F800044[0xB];
  done = 0;
  if (step != 0) {
    if (step == 1) {
      if (func_8014DAEC()) {
        func_8014D6B8(D_1F800044[8]);
        D_1F800044[0xB] = 2;
      }
    } else {
      func_8014D978();
      if (D_80143BB0 == 0) {
        done = 1;
      }
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
