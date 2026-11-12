#include "bof3/bof3.h"

/* Shared pad-state word: the pressed buttons of the current frame. */
extern u16 D_80145AA8; /* @source 0x80145AA8 @kind data */

/* Shared pressed-button masks paired with the pad-state word: 0x80145AC2 is the
 * confirm mask and 0x80145AC4 the cancel mask in this family's pad tests. */
extern u16 D_80145AC2; /* @source 0x80145AC2 @kind data */
extern u16 D_80145AC4; /* @source 0x80145AC4 @kind data */

/* Shared fairy progress byte, which the cancel path advances by one through the
 * array base. */
extern u8 fairyProgress[]; /* @source 0x801448EC @kind data */

/* Shared commu list cursor: the list row this overlay's handlers act on. */
extern u8 D_801448ED; /* @source 0x801448ED @kind data */

/* Current commu request: mode byte plus its two parameter bytes. */
extern u8 D_801D4DF4; /* @source 0x801D4DF4 @kind data */
extern u8 D_801D4DF8; /* @source 0x801D4DF8 @kind data */
extern u8 D_801D4DFC; /* @source 0x801D4DFC @kind data */

/* Snapshot of the request bytes above, taken here before the request is acted
 * on and restored by the cancel paths. */
extern u8 D_801D4E00; /* @source 0x801D4E00 @kind data */
extern u8 D_801D4E04; /* @source 0x801D4E04 @kind data */
extern u8 D_801D4E08; /* @source 0x801D4E08 @kind data */

/* Result halfword of the current commu request. */
extern u16 D_801D4E0C; /* @source 0x801D4E0C @kind data */

/* Ordinal byte of the current commu request: the zero-based slot ordinal of the
 * request row inside its record kind. */
extern u8 D_801D4E24; /* @source 0x801D4E24 @kind data */

/* Overlay-wide step cell group: byte 0x801F294C is the step this overlay's
 * handlers advance by one as their commu request is accepted, and 0x801F294D is
 * the commu request step byte the confirm paths set and the cancel paths roll
 * back. */
extern u8 D_801F294C; /* @source 0x801F294C @kind data */
extern u8 D_801F294D; /* @source 0x801F294D @kind data */

/* Pending world id; the commu entry count of a world is one byte of the table at
 * 0x801F286D indexed by it. */
extern u16 D_80143F00; /* @source 0x80143F00 @kind data */
extern u8 D_801F286D[]; /* @source 0x801F286D @kind data */

/* @source 0x8015DF18 @kind unknown
 * Resident executable sound-cue dispatcher; consumes one u32 cue ID.
 */
extern void dispatchSoundCue(u32 cue_id);

void publishCommuEntryTextId(void);
u8 countActiveRecordsByKind(s32 kind);
extern void moveCommuRowCursorByPadDirection(u8 *rowPtr);
extern void drawRowMarkerOutlineCommu0107_801D36FC(u8 row, u8 selected);
extern void drawCommuEntryListScreen(void);

/* @source 0x801D0CAC
 * @behavior Handler at entry 1 of the run at 0x801D4BCC. Reads the shared
 * pad-state word 0x80145AA8 once and dispatches on its masks. While that word
 * shares a bit with the cancel mask 0x80145AC4 it reports result code 0xFFFF
 * through the request result halfword 0x801D4E0C and advances the shared fairy
 * progress byte 0x801448EC by one. While it shares a bit with the confirm mask
 * 0x80145AC2 it snapshots the current commu request 0x801D4DF4, 0x801D4DF8 and
 * 0x801D4DFC into 0x801D4E00, 0x801D4E04 and 0x801D4E08, clears the commu
 * request step byte 0x801F294D and the request ordinal byte 0x801D4E24, and
 * accepts the request only when the mode byte allows it: mode 1 requires that
 * the world commu entry count, the byte of the table at 0x801F286D indexed by
 * the world id halfword 0x80143F00, is not below the commu list row 0x801448ED
 * read as an unsigned byte, and modes 0 and 2 require at least one in-use
 * active record of the kind named by that list row. An accepted request queues
 * resident sound cue 0x104 through the shared cue dispatcher at 0x8015DF18,
 * publishes the request text id through publishCommuEntryTextId and advances the
 * overlay step byte 0x801F294C by one; a rejected one, which is also every mode
 * value other than those three, queues cue 0x107 and leaves the step alone. When
 * neither mask matches it moves the commu list row through
 * moveCommuRowCursorByPadDirection. Every
 * path then runs drawCommuEntryListScreen and draws the list row 0x801448ED with the
 * selected flag 0 through drawRowMarkerOutlineCommu0107_801D36FC. Its 0x18-byte frame only holds ra.
 * @status exact
 * @match 100.00
 * @residual none
 */
void confirmCommuEntryRequestThenDrawCommu0107_801D0CAC(void) {
  u16 pad;

  pad = D_80145AA8;
  if (pad & D_80145AC4) {
    D_801D4E0C = 0xFFFF;
    fairyProgress[0] += 1;
  } else if (pad & D_80145AC2) {
    D_801F294D = 0;
    D_801D4E24 = 0;
    D_801D4E00 = D_801D4DF4;
    D_801D4E04 = D_801D4DF8;
    D_801D4E08 = D_801D4DFC;
    switch (D_801D4E08) {
    case 0:
    case 2:
      if (countActiveRecordsByKind(D_801448ED) != 0) {
        dispatchSoundCue(0x104);
        publishCommuEntryTextId();
        D_801F294C += 1;
      } else {
        dispatchSoundCue(0x107);
      }
      break;
    case 1:
      if ((u32)D_801F286D[D_80143F00] < (u32)D_801448ED) {
        dispatchSoundCue(0x107);
      } else {
        dispatchSoundCue(0x104);
        publishCommuEntryTextId();
        D_801F294C += 1;
      }
      break;
    default:
      dispatchSoundCue(0x107);
      break;
    }
  } else {
    moveCommuRowCursorByPadDirection(&D_801448ED);
  }
  drawCommuEntryListScreen();
  drawRowMarkerOutlineCommu0107_801D36FC(D_801448ED, 0);
}
