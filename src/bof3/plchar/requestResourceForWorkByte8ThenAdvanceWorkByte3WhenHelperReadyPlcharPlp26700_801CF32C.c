#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell. The original loads this cell for the
 * byte-8 argument and loads it again after the resource request, so the source
 * keeps the named cell form and evaluates it at both uses.
 * @source 0x1F800044 @kind data
 */
extern u8* D_1F800044;

/* Readiness poll helper of the main executable; the sibling plchar, world, boss
 * and bmagic lifts declare the same prototype. The original keeps only the
 * returned byte as the ready test, so the declared return type is that byte.
 */
extern u8 func_8014DAEC(void);

/* Shared main-executable resource request helper; the sibling plchar, world,
 * boss and bmagic lifts declare the same prototype and pass a resource id.
 */
extern void func_8014D6B8(u32 flag);

/* @source 0x801CF32C
 * @behavior Work-step handler for work byte 3 == 3 of the scratch work object
 * published at the scratchpad pointer cell 0x1F800044. Its only in-image
 * reference is the word 0x801CF32C at 0x801D09BC, entry 3 of this overlay's
 * code-pointer run based at 0x801D09B0, which the byte-3 dispatcher
 * func_801CEE6C indexes with that record byte scaled by four; no jal in this
 * payload names it, so it is reached through the run alone. It calls the shared
 * readiness helper func_8014DAEC and keeps only the byte that helper returns as
 * a ready test: while the helper reports not ready the handler writes nothing.
 * Once it reports ready the handler passes the record unsigned byte 8 to the
 * shared resource request helper func_8014D6B8, then re-loads the pointer cell
 * and advances the record unsigned byte 3, the work-step index, by one, so the
 * next dispatch reaches the word 0x801CF38C at 0x801D09C0. Takes no arguments
 * and returns nothing; reads or writes no other state, and the 0x18-byte frame
 * only keeps the return address at 0x10 across its two calls. The roles of
 * record byte 8 and of the requested resource are not established here, so the
 * name describes the effect.
 * @status exact
 * @match 100.00
 * @residual none
 */
void requestResourceForWorkByte8ThenAdvanceWorkByte3WhenHelperReadyPlcharPlp26700_801CF32C(
    void) {
  if (func_8014DAEC()) {
    func_8014D6B8(D_1F800044[8]);
    D_1F800044[3]++;
  }
}
