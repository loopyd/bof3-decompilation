#include "bof3/bof3.h"

/* Leading byte of the main-RAM game-mode record; the original reads it with a
 * single `lbu`, so the cell keeps its byte width. */
extern u8 D_80145E98; /* @source 0x80145E98 @kind data */

/* Signed scenario-state byte (the first byte of the etc/game scenario-state
 * record), taken with `lb` and tested against 8, so the declaration keeps the
 * s8 width this target's initArea source already gives it. */
extern s8 D_80146870; /* @source 0x80146870 @kind data */

/* Shared state-entry selector: dispatchIndexedStateHandler indexes the
 * 0x801C84BC handler table with this byte and the world overlays publish their
 * next state entry here. The original stores it with one `sb`. */
extern u8 D_801448EB; /* @source 0x801448EB @kind data */

/* @source 0x801F3378
 * @behavior Selects the shared state-entry byte 0x801448EB from the signed
 *   scenario-state byte 0x80146870 once the overlay request matches: the
 *   function returns 0 unless the first two zero-extended u16 arguments read
 *   0x3F and 0xE and the leading byte of the main-RAM game-mode record
 *   0x80145E98 reads 1. On a match it stores 0x1A when the scenario-state byte
 *   is at least 8 and 6 when it is below 8, then returns 1. Both arguments
 *   arrive masked with `andi ..., 0xFFFF`; no call and no frame.
 *
 * Name evidence: 0x801448EB is the state selector that game00 dispatches the
 *   handler table 0x801C84BC through (dispatchIndexedStateHandler reads it as
 *   an s8 table index) and that the world overlays set as their state entry;
 *   0x80146870 is the first byte of the etc/game/00 scenario-state record
 *   (scenarioState, declared s8 at the original `lb` width in
 *   include/bof3/ui/shop00_internal.h); 0x80145E98 is the leading byte of the
 *   main-RAM game-mode record table, read here with `lbu`. The two arguments
 *   carry no evidenced role, so the name keeps them structural.
 *
 * Shape: this payload's span at offset 0x778 (108 bytes) compares equal to the
 *   spans at the same offset in world04 areas 176, 177, 178, 179, 180, 181,
 *   182, 183 and 185, one `cmp` of 108 bytes per twin. Area179/13 carries an
 *   exact source over the same bytes, and this body copies its statement order,
 *   operand forms and declaration style; the span holds no address-specific
 *   immediate, so the guard constants 0x3F/0xE/1 and the stored values 0x1A/6
 *   are read from this payload's own bytes.
 * @status exact
 * @match 100.00
 * @residual none
 */
s32 selectStateMatchedArguments(u16 arg0,
                                                                u16 arg1) {
  if (arg0 != 0x3F) {
    return 0;
  }
  if (arg1 != 0xE) {
    return 0;
  }
  if (D_80145E98 == 1) {
    if (D_80146870 >= 8) {
      D_801448EB = 0x1A;
    } else {
      D_801448EB = 6;
    }
    return 1;
  }
  return 0;
}
