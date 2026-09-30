#include "bof3/bof3.h"

extern u8 D_80146874;
extern u8 D_80146875;

void func_8015C088(void);
void func_8015DF18(u16 arg0);

/* @source 0x801FD450
 * @behavior Overlay state-request handler: it first runs the shared front-end
 * startup helper func_8015C088, then requests primary scene state 0x10 with
 * secondary sub-state 0x0F by storing 0x0F in the shared sub-state byte
 * D_80146875 and 0x10 in the shared state byte D_80146874, and finally queues
 * the front-end cue 0x106 through func_8015DF18(u16). It takes no arguments
 * and returns nothing; its 0x18-byte frame only keeps $ra across the calls,
 * each of the two byte stores materialises the 0x8014 page in $at on its own
 * (which is how the psyq compiler emits separate shared-byte stores), and the
 * cue argument is materialised as `addiu $a0,$zero,0x106` in the `jal`
 * delay slot of the final call. No in-image `jal` targets the address; the
 * only in-image reference is the pointer word at 0x801FE344, entry 16 of the
 * overlay callback table D_801FE304 that
 * dispatchRecordCallbackByByte7AScenarioScena0200_801FD084 (0x801FD084)
 * indexes with a record's byte field 0x7A, so the overlay reaches this
 * handler only indirectly. Its sub-state-then-state store shape is the one
 * used by the sibling lifts
 * requestPrimaryState1BSubstate10ScenarioScena0200_801FD53C (0x801FD53C,
 * which stores 0x0A then 0x1B) and
 * requestPrimaryState1BSubstate5ScenarioScena0200_801FD6BC (0x801FD6BC, which
 * stores 5 then 0x1B); this member differs by storing 0x0F then 0x10 and by
 * additionally queuing cue 0x106 afterwards.
 * @status exact
 * @match 100.00
 * @residual none
 */
void requestPrimaryState10Substate0FAndQueueCue106ScenarioScena0200_801FD450(void) {
  func_8015C088();
  D_80146875 = 0xF;
  D_80146874 = 0x10;
  func_8015DF18(0x106u);
}
