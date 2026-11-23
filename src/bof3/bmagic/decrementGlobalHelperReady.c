#include "bof3/bof3.h"

/* Shared battle work record pointer in main RAM, outside this overlay's image.
 * Splat binds the address through this target's generated
 * undefined_syms_auto.txt row D_801EB4E0 = 0x801EB4E0, so no target-map row is
 * needed. The exact sibling lift
 * decrementWorkMotionWord44By1000ThenDecrementGlobalWorkByteBAndInvokeHelper
 * WhenTintChannel5FIsMinus80BmagicMagic15403_801EF454 reads the same record
 * through this pointer and decrements its global work byte 0xB the same way
 * this body decrements byte 9. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Scratchpad work-object pointer cell at 0x1F800044, outside this overlay's
 * image. This target's map binds no name there, so the cell is reached through
 * its address-anchored name, the same spelling the exact siblings
 * seedWorkByte9FromHelperAFF98AndAdvanceWorkByte2BmagicMagic15403_801EF544 and
 * decrementWorkMotionWord44By1000ThenDecrementGlobalWorkByteBAndInvokeHelper
 * WhenTintChannel5FIsMinus80BmagicMagic15403_801EF454 bind it with. The
 * original reads the cell after the byte 9 store, so this body keeps the named
 * cell form and evaluates it at that use. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind data */

/* Readiness poll helper of the shared main executable, bound by this target's
 * generated undefined_funcs_auto.txt row func_8014DAEC = 0x8014DAEC. The
 * original keeps only the low byte of the returned word as the guard
 * (andi $v0,0xFF), so the declared return type is that byte, the prototype the
 * exact sibling lift requestResourceForWorkByte8ThenAdvanceWorkByte3
 * WhenHelperReadyPlcharPlp03400_801CF84C measures with the same call sequence. */
extern u8 func_8014DAEC(void); /* @source 0x8014DAEC @kind function */

/* @source 0x801EF648
 * @behavior Work byte 2 step handler of this overlay's work object, entry 2 of
 *           the four-entry work byte 2 handler table at 0x801EEC14 (0x801EF544,
 *           0x801EF58C, 0x801EF648, 0x801EF6AC) that the exact sibling
 *           dispatchWorkByte2HandlerThenInvokeD290WhenWorkByte0Nonzero
 *           BmagicMagic15403_801EF4B4 copies into its own frame and invokes
 *           with no arguments and no bounds test, so the payload holds no jal
 *           to this address. It calls the shared readiness helper
 *           func_8014DAEC and keeps only that helper's returned byte as the
 *           test (jal / andi $v0,0xFF / beqz), so while the helper reports not
 *           ready the handler writes nothing. Once it reports ready the handler
 *           decrements the shared battle work record's byte 9 by one at byte
 *           width (lbu / addiu -1 / sb) through the pointer at 0x801EB4E0 and
 *           then re-reads the scratchpad work-object pointer cell at 0x1F800044
 *           and increments the work object's unsigned byte 2 by one, advancing
 *           the work byte 2 step index. The 0x18-byte frame keeps only $ra at
 *           0x10; it takes no arguments, returns nothing and reads or writes no
 *           other state. The role of record byte 9 is not established in this
 *           payload, so the name describes the writes.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementGlobalHelperReady(
    void) {
  if (func_8014DAEC()) {
    D_801EB4E0[9]--;
    D_1F800044[2]++;
  }
}
