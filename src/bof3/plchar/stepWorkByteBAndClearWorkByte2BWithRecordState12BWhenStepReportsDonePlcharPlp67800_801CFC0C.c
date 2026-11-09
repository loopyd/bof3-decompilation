#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell of the shared main executable. The body
 * reads the dispatch byte through one load taken ahead of frame creation and
 * re-evaluates the cell after the readiness poll, before the resource request
 * argument read, and again before the byte-0x0B store, because a call or a
 * store through the pointer may change the record the cell points at, so the
 * source keeps the named-cell form and evaluates it at each access the original
 * reloads. The exact lifts of this same target bind the cell the same way.
 * @source 0x1F800044 @kind data
 */
extern u8* D_1F800044;

/* State byte of the shared main executable that gates the done report of this
 * step: the third arm reports done only while this byte reads zero, so a
 * nonzero state suppresses the two clears. The exact sibling lifts of this body
 * read the same byte and none of them establishes its role, so it stays
 * address-anchored. This target generated undefined_syms_auto.txt binds the
 * address.
 * @source 0x80143BB0 @kind unknown
 */
extern u8 D_80143BB0;

/* Front-end record pointer cell of the shared main executable. The body clears
 * byte 0x12B of the record this cell publishes when the step reports done. The
 * exact lifts of this target publish their work record through the same cell
 * and its role here is not established, so it stays address-anchored. This
 * target generated undefined_syms_auto.txt binds the address.
 * @source 0x80146250 @kind unknown
 */
extern u8* D_80146250;

/* Shared readiness helper of the shared main executable, outside this payload:
 * it answers an unsigned byte and both polling arms of this body test only that
 * byte for zero. This target generated undefined_funcs_auto.txt binds the
 * address, and the exact lifts of this target call it the same way.
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
void func_8014D978(void);

/* Shape note: the 0xEC-byte body carries the same instructions as the exact
 * lifted bodies of ten other plchar overlays, which carry the same effect name:
 * plp012, plp015, plp016, plp025 and plp026 at 0x801CEC30, plp27a 0x801CF35C,
 * plp247 0x801CF578, plp257 0x801CF958, plp478 0x801CED80 and plp567
 * 0x801D0208. A payload-to-payload comparison of this span against those bytes
 * shows each of them differs in exactly two words, the `j` words at span
 * offsets 0x68 and 0x8C that name the body own interior label and therefore
 * encode the function base address, and each difference equals the base-address
 * delta between the two spans, so this is relocation-different shape evidence
 * and not a clone: a scan of every binary under out/binaries finds no other
 * occurrence of these 0xEC bytes. The statement order, the single dispatch-byte
 * read, the reload-per-access pointer-cell form and the declaration style carry
 * over with this function own address.
 */
/* @source 0x801CFC0C
 * @behavior Work-step handler of the scratch work object read through the
 *           pointer cell 0x1F800044, reached from outside this payload: the
 *           body address appears once in the payload, as the word 0x801D05AC,
 *           entry 18 of the thirty-word code-pointer run based at 0x801D0564
 *           that this overlay places as the textbin row at that base, and no
 *           jal or j word of the payload names it, so it takes no arguments and
 *           any result is discarded. The body dispatches on the unsigned byte
 *           0x0B of the work object and keeps one done flag across the
 *           dispatch, which is why the flag survives the calls in a
 *           callee-saved register. When byte 0x0B is zero the body only polls
 *           the shared readiness helper func_8014DAEC and reports done as soon
 *           as that helper answers nonzero. When byte 0x0B is one it polls the
 *           same helper and, once the helper answers nonzero, passes the work
 *           object unsigned byte 0x08 to the shared resource request helper
 *           func_8014D6B8 and writes 2 into byte 0x0B so the next dispatch
 *           takes the third arm; this arm never reports done. For every other
 *           value of byte 0x0B the body runs the shared no-argument update
 *           helper func_8014D978 and reports done whenever the state byte
 *           0x80143BB0 reads zero, so a nonzero state suppresses the clear.
 *           Both polling arms and the third arm leave their answer in the one
 *           flag, which is tested once. When the flag is set the handler clears
 *           the work-object byte 0x2B and then the state byte 0x12B of the
 *           front-end record published at 0x80146250, re-reading both pointer
 *           cells for the two stores. The roles of work byte 0x0B, of work byte
 *           0x2B and of the requested resource are not established here, so the
 *           name describes the observed step and keeps the address. The body
 *           returns nothing, the 0x18-byte frame only keeps $ra at 0x14 and the
 *           flag register at 0x10 across the calls, every arm joins on one
 *           epilogue, and the slots after the dispatch byte, after each tested
 *           helper answer, after the published-record load and the
 *           untaken-branch delay slots stay nops.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stepWorkByteBAndClearWorkByte2BWithRecordState12BWhenStepReportsDonePlcharPlp67800_801CFC0C(
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
