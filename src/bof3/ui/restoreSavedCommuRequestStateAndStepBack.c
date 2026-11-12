#include "bof3/bof3.h"

/* Current commu request of the commu01/07 entry handlers: kind byte plus its
 * two parameter bytes, published by the entry handlers and read by the confirm
 * paths. */
extern u8 D_801D4DF4; /* @source 0x801D4DF4 @kind data */
extern u8 D_801D4DF8; /* @source 0x801D4DF8 @kind data */
extern u8 D_801D4DFC; /* @source 0x801D4DFC @kind data */

/* Snapshot of the request bytes above: the confirm path at 0x801D0CAC copies
 * the current request into these cells before it acts on it. */
extern u8 D_801D4E00; /* @source 0x801D4E00 @kind data */
extern u8 D_801D4E04; /* @source 0x801D4E04 @kind data */
extern u8 D_801D4E08; /* @source 0x801D4E08 @kind data */

/* Commu request step byte the confirm paths set and the cancel path rolls back. */
extern u8 D_801F294D; /* @source 0x801F294D @kind data */

/* @source 0x8015DF18 @kind unknown
 * Resident executable sound-cue dispatcher; consumes one u32 cue ID.
 */
extern void dispatchSoundCue(u32 cue_id);

/* @source 0x801D47C4
 * @behavior Restores the commu request that the confirm path at 0x801D0CAC
 * snapshotted: copies the saved request byte 0x801D4E00 into 0x801D4DF4, the
 * saved byte 0x801D4E04 into 0x801D4DF8 and the saved byte 0x801D4E08 into
 * 0x801D4DFC, queues resident sound cue 0x106 through the shared cue dispatcher
 * at 0x8015DF18, and rolls the commu request step byte at 0x801F294D back by
 * one. It takes no arguments and returns nothing, its 0x18-byte frame only holds
 * ra across the cue call, and the entry list handler at 0x801D1630 takes it on
 * its cancel path while 0x801D1158 is its other caller.
 * @status exact
 * @match 100.00
 * @residual none
 */
void restoreSavedCommuRequestStateAndStepBack(void) {
  D_801D4DF4 = D_801D4E00;
  D_801D4DF8 = D_801D4E04;
  D_801D4DFC = D_801D4E08;
  dispatchSoundCue(0x106);
  D_801F294D = D_801F294D - 1;
}
