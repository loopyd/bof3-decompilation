#include "bof3/bof3.h"

/* Shared work-counter word of the main executable that the battle hooks
 * accumulate. This body stores it as the slot 1 limit, derives the slot 4 limit
 * from it by dividing it by twenty, and keeps it in a register when it needs it
 * twice. */
extern u32 D_801FE4E8; /* @source 0x801FE4E8 @kind unknown */

/* Shared work-counter byte of the main executable, read here with a
 * zero-extending `lbu` and stored into the two limit slots that take a small
 * count. */
extern u8 D_801FE4EF; /* @source 0x801FE4EF @kind unknown */

/* Shared work-counter word of the main executable of which this body only
 * reads the whole word: it is the slot 2 limit and the value whose class picks
 * the slot 5 limit. */
extern u32 D_801FE4F0; /* @source 0x801FE4F0 @kind unknown */

/* Shared 164-byte field record of the main executable this overlay edits. Its
 * base is materialized once and kept in a saved register across the six
 * adjuster calls below, each of which takes the address of one of its
 * halfword fields. */
extern u8 D_80144DE4[]; /* @source 0x80144DE4 @kind data */

/* Six per-slot ramp limits of this overlay, filled by this body from the three
 * shared work counters and read by the sibling handler
 * stepSixRampPad as the limit each slot
 * value ramps towards. Three of the six are also the values this body pushes
 * into the shared field record. It sits inside the trailing data blob of the
 * payload, so the generated map already names it. */
extern u32 D_801D8EE8[]; /* @source 0x801D8EE8 @kind data */

/* Six per-slot ramp values of this overlay, zeroed here through a byte counter
 * and stepped towards the limits above by the sibling handler. It sits inside
 * the trailing data blob of the payload, so the generated map already names
 * it. */
extern u32 D_801D8F00[]; /* @source 0x801D8F00 @kind data */

/* Main-exe halfword adjuster at 0x801654F4, the span the shared header
 * bof3/core/slus_functions.h reviews as adjustHalfwordWith999Cap: it adds the
 * signed value to the halfword field and saturates that field at 999 and 0,
 * reporting the absorbed amount through an s16 return this body discards. That
 * canonical name has no binding in this target's map, so the raw address name
 * carries the two calls here. */
extern s16 func_801654F4(u16 *target, s32 value); /* @source 0x801654F4 @kind unknown */

/* Main-exe halfword adjuster at 0x8016546C, called exactly like the reviewed
 * adjuster above with the address of a record halfword field and a
 * zero-extended value, and with a result this body discards. No span in this
 * repository reviews it, so its name stays anchored at its address. */
extern s16 func_8016546C(u16 *target, s32 value); /* @source 0x8016546C @kind unknown */

/* @source 0x801D259C
 * @behavior Seeds the six-slot ramp state of the BATE overlay from the shared
 *           battle work counters once a phase handler reaches it. It first
 *           zeroes the six per-slot ramp values D_801D8F00. It then fills the
 *           six per-slot ramp limits D_801D8EE8 in the order the shipped
 *           stores show: slot 1 takes the work-counter word D_801FE4E8, slot 2
 *           the work-counter word D_801FE4F0, slot 0 and slot 3 the
 *           work-counter byte D_801FE4EF, slot 4 that word divided by twenty,
 *           and slot 5 the class the word D_801FE4F0 selects - 2 below 0x32, 4
 *           from 0x32 up to 0x50 and 6 from 0x50 up, the middle arm tested as
 *           the compound range whose shipped bytes are one unsigned range
 *           check. Three of those limits
 *           reach the shared field record D_80144DE4: the slot 3 limit after a
 *           sign-extending halfword read at the record field 0x42 and again at
 *           0x22 through the main-exe 999-cap adjuster 0x801654F4, the slot 4
 *           limit as a zero-extended halfword at the fields 0x3C and 0x1C
 *           through func_8016546C, and the slot 5 limit after a sign-extending
 *           halfword read at the fields 0x40 and 0x20 through 0x801654F4. The
 *           record base and the address of the slot 3 limit are each
 *           materialized once and kept across those six calls; every adjuster
 *           answer is discarded and nothing is returned. The 0x20-byte frame
 *           keeps the return address at 0x18 and the two saved registers at
 *           0x10 and 0x14.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateSixRampLimits(void) {
  u8 i;
  u8 *record;
  s16 *limit;

  for (i = 0; i < 6; i++) {
    D_801D8F00[i] = 0;
  }
  D_801D8EE8[0] = D_801FE4EF;
  D_801D8EE8[1] = D_801FE4E8;
  D_801D8EE8[2] = D_801FE4F0;
  D_801D8EE8[3] = D_801FE4EF;
  D_801D8EE8[4] = D_801FE4E8 / 20;
  if (D_801FE4F0 < 0x32) {
    D_801D8EE8[5] = 2;
  } else if (D_801FE4F0 >= 0x32 && D_801FE4F0 < 0x50) {
    D_801D8EE8[5] = 4;
  } else if (D_801FE4F0 >= 0x50) {
    D_801D8EE8[5] = 6;
  }
  record = D_80144DE4;
  limit = (s16 *)&D_801D8EE8[3];
  func_801654F4((u16 *)(record + 0x42), *limit);
  func_801654F4((u16 *)(record + 0x22), *limit);
  func_8016546C((u16 *)(record + 0x3C), *(u16 *)&D_801D8EE8[4]);
  func_8016546C((u16 *)(record + 0x1C), *(u16 *)&D_801D8EE8[4]);
  func_801654F4((u16 *)(record + 0x40), *(s16 *)&D_801D8EE8[5]);
  func_801654F4((u16 *)(record + 0x20), *(s16 *)&D_801D8EE8[5]);
}
