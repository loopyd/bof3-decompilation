#include "bof3/bof3.h"

/* @source 0x8015DF18 @kind unknown
 * Resident executable sound-cue dispatcher; consumes one u32 cue ID.
 */
extern void dispatchSoundCue(u32 cue_id);

/* Overlay step byte this overlay's handlers advance or roll back as their commu
 * request completes. */
extern u8 D_801F294D; /* @source 0x801F294D @kind data */

/* Result halfword of the current commu request. */
extern u16 D_801D4E0C; /* @source 0x801D4E0C @kind data */

/* Saved step byte and saved result halfword, written here and read back by the
 * cancel path at 0x801D2110. */
extern u8 D_801D4E10; /* @source 0x801D4E10 @kind data */
extern u16 D_801D4E14; /* @source 0x801D4E14 @kind data */

/* Toggle byte the confirm path of the same handler run flips. */
extern u8 D_801D4E18; /* @source 0x801D4E18 @kind data */

/* @source 0x801D4B58
 * @behavior Overlay state save that the confirm paths run when the selected
 * commu entry row has no target cell (the row cell byte reads 0xFF): copies the
 * overlay step byte at 0x801F294D into 0x801D4E10 and the request result
 * halfword at 0x801D4E0C into 0x801D4E14, queues resident sound cue 0x103
 * through the shared cue dispatcher at 0x8015DF18, publishes request result
 * 0x3D into 0x801D4E0C, clears the byte at 0x801D4E18 and selects overlay step
 * 5 at 0x801F294D. It takes no arguments and returns nothing, its 0x18-byte
 * frame only holds ra across the cue call, and the saved pair is what the
 * cancel path at 0x801D2110 restores.
 * @status exact
 * @match 100.00
 * @residual none
 */
void saveCommuRequestStateAndSelectStep5(void) {
  u8 step;
  u16 result;

  step = D_801F294D;
  result = D_801D4E0C;
  D_801D4E10 = step;
  D_801D4E14 = result;
  dispatchSoundCue(0x103);
  D_801D4E0C = 0x3D;
  D_801D4E18 = 0;
  D_801F294D = 5;
}
