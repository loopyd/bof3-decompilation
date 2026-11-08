#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell. The original loads this cell for the
 * byte-8 argument and loads it again after the resource request, so the source
 * keeps the named cell form and evaluates it at both uses.
 * @source 0x1F800044 @kind data
 */
extern u8* D_1F800044;

/* Readiness poll helper of the main executable; the battle and bmagic lifts
 * declare the same prototype. The original keeps only the returned byte as the
 * ready test, so the declared return type is that byte.
 */
extern u8 func_8014DAEC(void);

/* Shared main-executable resource request helper; the sibling world, boss and
 * bmagic lifts declare the same prototype and pass a resource id.
 */
extern void func_8014D6B8(u32 flag);

/* @source 0x801D0054
 * @behavior Work byte 3 == 3 step of the work-byte-3 dispatcher at 0x801CFB94.
 *           This function is entry 3 of the thirty-word code-pointer run at
 *           0x801D0A54, the word 0x801D0054 at 0x801D0AA0; that dispatcher
 *           indexes the run from its base word 0x801D0A94 with work-record byte
 *           3 read through the scratchpad pointer cell 0x1F800044. No jal in
 *           this payload names this function, so the run is its only in-image
 *           reference. It calls the shared readiness helper func_8014DAEC and
 *           keeps only the returned byte as the ready test: while that byte
 *           reads zero it writes nothing. Once it reads nonzero it passes the
 *           record byte 8 to the shared resource request helper func_8014D6B8,
 *           then re-loads the pointer cell and advances record byte 3, the
 *           work-step index, by one, so the next dispatch reaches the word
 *           0x801D00B4 at 0x801D0AA4. Takes no arguments and returns nothing;
 *           reads or writes no other state, and the 0x18-byte frame only keeps
 *           the return address at 0x10 across its two calls. The roles of record
 *           byte 8 and of the requested resource are not established here, so
 *           the name describes the effect.
 * @status exact
 * @match 100.00
 * @residual none
 */
void requestResourceForWorkByte8ThenAdvanceWorkByte3WhenHelperReadyPlcharPlp02600_801D0054(
    void) {
  if (func_8014DAEC()) {
    func_8014D6B8(D_1F800044[8]);
    D_1F800044[3]++;
  }
}
