#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell of the shared main executable. The body
 * reads it once for the byte it dispatches on and re-reads it before every
 * store, because a store through the pointer may alias the cell itself.
 * @source 0x1F800044 @kind data
 */
extern u8* D_1F800044;

/* Front-end record pointer cell of the shared main executable. The body clears
 * byte 0x12B of the record it publishes when the step reports done.
 * @source 0x80146250 @kind unknown
 */
extern u8* D_80146250;

/* Local state byte of the shared main executable: the front-end handlers that
 * own it treat 2 as busy and every other value as free. The generated
 * undefined_syms_auto.txt of this target binds it and the shared main-executable
 * map carries the same address.
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

/* @source 0x801CEC30
 * @behavior Work-step handler of the front-end work object read through the
 *           scratchpad pointer cell 0x1F800044. Its only in-image reference is
 *           the word 0x801CFD94, entry 5 of this payload's sixteen-word
 *           code-pointer run based at 0x801CFD80; no jal in this payload names
 *           it, so it is reached through the run alone. The body dispatches on
 *           the unsigned byte 0x0B of the work object and keeps one "done" flag
 *           across the dispatch, which is why the flag has to survive the calls
 *           in a callee-saved register. When byte 0x0B is zero the body only
 *           polls the shared readiness helper func_8014DAEC and reports done as
 *           soon as that helper answers nonzero. When byte 0x0B is one it polls
 *           the same helper and, once the helper answers nonzero, passes the
 *           work-object unsigned byte 0x08 to the shared resource request helper
 *           func_8014D6B8 and writes 2 into byte 0x0B so the next dispatch takes
 *           the third arm; this arm never reports done. For every other value of
 *           byte 0x0B the body runs the shared no-argument update helper
 *           func_8014D978 and reports done whenever the local state byte
 *           0x80143BB0 reads zero, so a busy state of that byte suppresses the
 *           clear. Both the polling arms and the third arm leave their answer in
 *           the one flag, which is tested once. When the flag is set the handler
 *           clears the work-object byte 0x2B and then clears the state byte
 *           0x12B of the front-end record published at 0x80146250, re-reading
 *           both pointer cells for the stores. It takes no arguments, returns
 *           nothing and answers no value: the set path leaves the loaded record
 *           pointer in the return register, but the original discards it. The
 *           0x18-byte frame only keeps $ra at 0x14 and the flag register at 0x10
 *           across the calls, and every arm joins on one epilogue. The span is
 *           0xEC bytes at payload offset 0x830. The roles of work byte 0x0B, of
 *           work byte 0x2B and of the requested resource are not established
 *           here, so the name describes the observed step and keeps the address.
 *           The same span in the plp27a/00 payload, at 0x801CF35C, is identical
 *           in 234 of its 236 bytes: the two differing words are its own two
 *           jumps to the shared join, which encode that function's own base
 *           address. A relocation difference is shape evidence only, so this
 *           body is seeded from that lift's statement order and declarations
 *           and is not a clone of it.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stepWorkByteBAndClearWorkByte2BWithRecordState12BWhenStepReportsDonePlcharPlp01200_801CEC30(
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
