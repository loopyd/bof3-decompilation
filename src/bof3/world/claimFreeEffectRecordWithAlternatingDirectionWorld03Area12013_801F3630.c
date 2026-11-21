#include "bof3/bof3.h"

/* The alternating direction byte this claim pass hands to every effect record
 * it takes. It sits in this overlay's own payload, one word below the
 * work-byte-2 handler table that starts at 0x801F43E8, so it is this target's
 * cell and is declared here at its own address rather than through a shared
 * row. */
extern u8 D_801F43E4; /* @source 0x801F43E4 @kind data */

/* One 0x1C-byte entry of the 0x100-entry effect table at 0x800E4800, the layout
 * the exact per-frame sweep of this same target models: the lead byte marks an
 * entry live, the three bytes behind it hold the frame countdown, the state its
 * switch dispatches on and the direction flag that picks the sign of the
 * offsets each state applies. Only that head is written here, and the trailing
 * pad carries the stride so the walk advances by 0x1C. */
typedef struct Area12013EffectRecord {
  u8 flags_00;
  u8 timer_01;
  u8 state_02;
  u8 dir_03;
  u8 pad_04[0x18];
} Area12013EffectRecord;

/* @source 0x801F3630
 * @behavior Effect-record claim pass: it walks the lead byte of each of the
 *           0x100 entries of the 0x1C-byte-stride effect table at 0x800E4800,
 *           the work area this overlay shares with the world and scenario
 *           overlays, and takes the first entry whose lead byte reads zero.
 *           Taking one writes 1 into that lead byte, clears the state byte at
 *           +0x02, copies this overlay's alternating direction byte at
 *           0x801F43E4 into the entry byte +0x03 and then flips that byte, so
 *           consecutive claims alternate. The countdown byte at +0x01 and every
 *           word of the entry keep their values. The body calls nothing, takes
 *           no arguments and keeps no register, so its 0x80 bytes carry no
 *           prologue and no frame; the adjacent byte pair at +0x02 and +0x03 is
 *           written through a second walk base the compiler materialises at
 *           entry + 3, at -0x1 and 0. The bound test is the loop's back edge:
 *           the unsigned 16-bit counter is read back masked to 0xFFFF and
 *           compared below 0x100, the 0x1C advance of the entry pointer is
 *           scheduled into that back edge's delay slot, and the claim leaves the
 *           loop through the lead-byte branch, which jumps over the exhaustion
 *           path to the bare return. The claim path leaves the flipped byte in
 *           $v0; the payload's only call sites, the four consecutive jal
 *           instructions at 0x801F2CBC, 0x801F2CC4, 0x801F2CCC and 0x801F2CD4
 *           inside
 *           step1ClaimFourEffectRecordsSweepArmWorkByte6To1AndAdvanceWorkByte1OnCountdownExpiryWorld03Area12013_801F2CB4,
 *           read no result, so the body returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void claimFreeEffectRecordWithAlternatingDirectionWorld03Area12013_801F3630(void) {
  Area12013EffectRecord* record;
  u16 count;

  record = (Area12013EffectRecord *)PSX_PTR(u8, 0x800E4800u);

  for (count = 0u; count < 0x100u; count++) {
    if (record->flags_00 == 0) {
      record->flags_00 = 1;
      record->state_02 = 0;
      record->dir_03 = D_801F43E4;
      D_801F43E4 ^= 1;
      break;
    }
    record++;
  }
}
