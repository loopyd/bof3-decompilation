#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell. The original loads this cell for the
 * byte-8 argument and loads it again after the resource request, so the source
 * keeps the named cell form and evaluates it at both uses.
 * @source 0x1F800044 @kind data
 */
extern u8* D_1F800044;

/* Readiness poll helper of the main executable; the sibling world, battle and
 * bmagic lifts declare the same prototype. The original keeps only the returned
 * byte as the ready test, so the declared return type is that byte.
 */
extern u8 func_8014DAEC(void);

/* Shared main-executable resource request helper; the sibling world, boss and
 * bmagic lifts declare the same prototype and pass a resource id.
 */
extern void func_8014D6B8(u32 flag);

/* @source 0x801CF84C
 * @behavior Overlay step reached through the code-pointer word 0x801CF84C at
 *           0x801D032C, entry 14 of this overlay's twenty-eight-word handler run
 *           at 0x801D02F4 that the byte-3 dispatcher
 *           dispatchWorkByte3Table0320PlcharPlp03400_801CF37C indexes from its
 *           base word 0x801D0320 with work-record byte 3 scaled by four, so the
 *           body runs as that dispatcher's step 3; no jal in this payload names
 *           it, so it is reached through the run alone. It calls the shared
 *           readiness helper func_8014DAEC and keeps only that helper's returned
 *           byte as a ready test: while the helper reports not ready the handler
 *           writes nothing. Once it reports ready the handler passes the work
 *           record's unsigned byte 8 to the shared main-executable resource
 *           request helper func_8014D6B8, then re-loads the scratchpad pointer
 *           cell 0x1F800044 and advances the record's unsigned byte 3, the
 *           work-step index, by one, so the next dispatch reaches the word
 *           0x801CF8AC at 0x801D0330. Takes no arguments and returns nothing,
 *           reads or writes no other state, and the 0x18-byte frame only keeps
 *           the return address at 0x10 across its two calls. The roles of
 *           record byte 8 and of the requested resource are not established
 *           here, so the name describes the effect.
 * @status exact
 * @match 100.00
 * @residual none
 */
void requestResourceForWorkByte8ThenAdvanceWorkByte3WhenHelperReadyPlcharPlp03400_801CF84C(
    void) {
  if (func_8014DAEC()) {
    func_8014D6B8(D_1F800044[8]);
    D_1F800044[3]++;
  }
}
