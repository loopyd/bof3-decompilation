#include "bof3/bof3.h"

typedef void (*Boss05516Handler)(void);

/* Phase-advance helper of this overlay: when bit 0x4 of the
 * main-program halfword 0x801462E8 is set it writes work-record byte +0xB = 1,
 * the halfword 0x801463BE = 8 and work-record byte +2 = 3 (or, when the
 * halfword 0x801463C0 is not 0x81, calls 0x801E567C instead); otherwise it
 * returns immediately. The store of 3 is one past the last step index this
 * dispatcher accepts, so the call below ends the step sequence. */
void endStepSequenceRaisePendingFlagBossBoss05516_800C227C(void);

/* Scratchpad pointer cell holding this overlay work record. Declared as the
 * absolute-address symbol so every use reloads the cell with the lui plus
 * offset load form the original shows. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/*
 * This overlay's handler run at 0x800C35D4 holds sixteen code pointers, entry 0
 * = 0x800C204C through entry 15 = 0x800C2FFC, and the four bases inside it are
 * used by four dispatchers: 0x800C35D8 (entry 1) here, 0x800C35EC (entry 6) in
 * dispatchIndexedHandlerBossBoss05516_800C22F0, and 0x800C35F8 (entry 9) in
 * publishEnemyWorkBaseThenDispatchStateByte1BossBoss05516_800C26D4. From this
 * base the reached entries are 0x800C2120, 0x800C2158, 0x800C21E4, 0x800C2234
 * and 0x800C2244, the five state bodies of this overlay. The Splat `textbin`
 * row T_800C35D4 places the words, so the run is only referenced here.
 */
extern Boss05516Handler D_800C35D8[]; /* @source 0x800C35D8 @kind table */

/* @source 0x800C20A4
 * @behavior Overlay entry dispatch on the state byte of the work record
 * published at scratchpad pointer cell 0x1F800044: calls the handler selected
 * by the record's unsigned byte +2 from this overlay's handler run based at
 * 0x800C35D8, then, when the record's byte +1 is exactly 8 and byte +2 is below
 * three, calls the phase-advance helper 0x800C227C. Both reads of the work
 * record are fresh loads around the indirect call, the index is not bounded
 * before the scaled table load, and nothing but $ra lives in the 0x18-byte
 * frame. The word at 0x800C358C holds this function's address, so it is an
 * entry of the mixed overlay entry run at 0x800C356C rather than of the handler
 * run it indexes; the prologue and return path alone place the boundary.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchStateByte2ThenAdvanceOnState8BossBoss05516_800C20A4(void) {
  D_800C35D8[((u8 *)g_battle_work)[2]]();

  if (((u8 *)g_battle_work)[1] == 8 && ((u8 *)g_battle_work)[2] < 3u) {
    endStepSequenceRaisePendingFlagBossBoss05516_800C227C();
  }
}
