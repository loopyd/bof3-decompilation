#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell. The original loads this cell for the
 * byte-8 argument and loads it again after the resource request, so the source
 * keeps the named cell form and evaluates it at both uses.
 * @source 0x1F800044 @kind data
 */
extern u8* D_1F800044;

/* Readiness poll helper of the main executable; the sibling plchar lifts of this
 * body declare the same prototype. The original keeps only the returned byte as
 * the ready test, so the declared return type is that byte.
 */
extern u8 func_8014DAEC(void);

/* Shared main-executable resource request helper; the sibling plchar lifts of
 * this body declare the same prototype and pass a resource id.
 */
extern void func_8014D6B8(u32 flag);

/* Seeded from the byte-identical lift of the same body at plp567 0x801CF130,
 * which measured exact before this seed was written. A cmp of this 0x60-byte
 * span against that member's range in both original payloads matched every
 * byte, so the statement order, the two pointer-cell evaluations and the
 * declaration style carry over with this function's own address, name and
 * dispatch facts.
 */
/* @source 0x801CED50
 * @behavior Work byte 3 == 3 step of this overlay: the word 0x801CED50 sits at
 *           0x801D03D8, index 3 of the run based at 0x801D03CC that this
 *           target's byte-3 dispatcher 0x801CE890 indexes with work byte 3
 *           scaled by four, and entry 11 of the twenty-eight-word handler run
 *           based at 0x801D03AC, so no jal in this payload names it. It calls
 *           the shared readiness helper func_8014DAEC and keeps only the byte
 *           that helper returns as a ready test, so while the helper reports
 *           not ready the body writes nothing. Once it reports ready the body
 *           passes the work record's unsigned byte 8, read through the
 *           scratchpad pointer cell 0x1F800044, to the shared resource request
 *           helper func_8014D6B8, then re-loads that cell and advances the
 *           record's unsigned byte 3, the work-step index, by one, which hands
 *           the dispatch to the next entry of the same run. It takes no
 *           arguments, returns nothing, reads or writes no other state, and the
 *           0x18-byte frame only keeps the return address at 0x10 across its
 *           two calls. The roles of record byte 8 and of the requested resource
 *           are not established here, so the name describes the effect.
 * @status exact
 * @match 100.00
 * @residual none
 */
void requestResourceForWorkByte8ThenAdvanceWorkByte3WhenHelperReadyPlcharPlp46700_801CED50(
    void) {
  if (func_8014DAEC()) {
    func_8014D6B8(D_1F800044[8]);
    D_1F800044[3]++;
  }
}
