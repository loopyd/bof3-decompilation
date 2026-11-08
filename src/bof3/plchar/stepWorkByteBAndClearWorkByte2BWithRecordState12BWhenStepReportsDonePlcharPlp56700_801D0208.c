#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell of the shared main executable. The body
 * reads it once ahead of frame creation for the byte it dispatches on and
 * re-evaluates the cell after the readiness poll and before the resource
 * request's argument read and before the byte-0x0B store, because a call or a
 * store through the pointer may change the record the cell points at, so the
 * source keeps the named-cell form and evaluates it again at each access the
 * original reloads.
 * @source 0x1F800044 @kind data
 */
extern u8* D_1F800044;

/* Front-end record pointer cell of the shared main executable. The body clears
 * byte 0x12B of the record this cell publishes when the step reports done. The
 * exact sibling lifts of this overlay publish their work record through the same
 * cell and its role here is not established, so it stays address-anchored.
 * @source 0x80146250 @kind unknown
 */
extern u8* D_80146250;

/* State byte of the shared main executable that gates the done report of this
 * step: the third arm reports done only while this byte reads zero, so a nonzero
 * state suppresses the two clears. The sibling lifts of the plchar overlays read
 * the same byte but the roles they give it differ, so it stays address-anchored.
 * @source 0x80143BB0 @kind unknown
 */
extern u8 D_80143BB0;

/* Shared readiness helper of the shared main executable, outside this payload:
 * it answers an unsigned byte and both polling arms of this body only test that
 * byte for zero. Splat binds its address through this target's generated
 * undefined_funcs_auto.txt, and the lifted siblings of this overlay call it the
 * same way.
 * @source 0x8014DAEC
 */
extern u8 func_8014DAEC(void);

/* Shared main-executable resource request helper. The body hands it the work
 * object's unsigned byte 0x08 as the requested id and discards any answer; the
 * exact sibling lift of this body in plp015/00 declares the same prototype.
 * @source 0x8014D6B8
 */
extern void func_8014D6B8(u32 id);

/* Shared no-argument update helper of the main executable, outside this payload;
 * the third arm of this body calls it and reads no result. The lifted siblings
 * of this overlay call it the same way.
 * @source 0x8014D978
 */
extern void func_8014D978(void);

/* @source 0x801D0208
 * @behavior Work-step handler of the scratch work object read through the
 *           pointer cell 0x1F800044. Its only in-image reference is the word
 *           0x801D0208 at 0x801D07EC, entry 0 of the seven-word code-pointer run
 *           that starts at 0x801D07EC and closes the payload, and no jal in this
 *           payload names it, so it is reached through that run alone and is an
 *           overlay entry called from outside the payload. The body dispatches on
 *           the unsigned byte 0x0B of the work object and keeps one done flag
 *           across the dispatch, which is why the flag survives the calls in a
 *           callee-saved register. When byte 0x0B is zero the body only polls the
 *           shared readiness helper func_8014DAEC and reports done as soon as
 *           that helper answers nonzero. When byte 0x0B is one it polls the same
 *           helper and, once the helper answers nonzero, passes the work
 *           object's unsigned byte 0x08 to the shared resource request helper
 *           func_8014D6B8 and writes 2 into byte 0x0B so the next dispatch takes
 *           the third arm; this arm never reports done. For every other value of
 *           byte 0x0B the body runs the shared no-argument update helper
 *           func_8014D978 and reports done whenever the state byte 0x80143BB0
 *           reads zero, so a nonzero state suppresses the clear. Both polling
 *           arms and the third arm leave their answer in the one flag, which is
 *           tested once. When the flag is set the handler clears the work-object
 *           byte 0x2B and then the state byte 0x12B of the front-end record
 *           published at 0x80146250, re-reading both pointer cells for the two
 *           stores. The roles of work byte 0x0B, of work byte 0x2B and of the
 *           requested resource are not established here, so the name describes
 *           the observed step and keeps the address. The body takes no arguments
 *           and returns nothing, the 0x18-byte frame only keeps $ra at 0x14 and
 *           the flag register at 0x10 across the calls, every arm joins on one
 *           epilogue, and the slots after the dispatch byte, after each tested
 *           helper answer, after the published-record load and the untaken-branch
 *           delay slots stay nops.
 *
 * Shape note: the 0xEC-byte body is instruction-for-instruction identical to the
 * exact lifted bodies of nine other plchar overlays, which carry the same effect
 * name: plp015 0x801CEC30, plp012 0x801CEC30, plp016 0x801CEC30, plp025
 * 0x801CEC30, plp026 0x801CEC30, plp27a 0x801CF35C, plp247 0x801CF578, plp257
 * 0x801CF958 and plp478 0x801CED80, so that sibling is the seed's evidence. A
 * payload-to-payload cmp of the 0xEC-byte spans shows each of them differs in
 * exactly two words, the `j` words at 0x801D0270 and 0x801D0294 that name the
 * body's own interior label and therefore encode the function's base address, so
 * this is relocation-different shape evidence and not a clone; the statement
 * order, the single dispatch-byte read, the reload-per-access pointer-cell form
 * and the declaration style carry over with this function's own address.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stepWorkByteBAndClearWorkByte2BWithRecordState12BWhenStepReportsDonePlcharPlp56700_801D0208(
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
