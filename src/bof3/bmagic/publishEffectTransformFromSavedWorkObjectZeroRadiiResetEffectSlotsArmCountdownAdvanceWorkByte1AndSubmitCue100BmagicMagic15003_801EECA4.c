#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell 0x1F800044, shared by all overlays and
 * already bound as g_battle_work by this target's map row. The original
 * re-materializes the cell base for each access (lui $v1,0x1F80 / lw
 * $v1,68($v1) before the byte-9 store and again before the byte-1 update), so
 * the cell stays the named symbol rather than a pointer local that would fold
 * the two reads into one. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared action-record pointer cell of the main executable at 0x801EC2E0,
 * outside this overlay's image and bound through this target's generated
 * undefined_syms_auto.txt. Word 0x74 of that record is a saved work-object
 * pointer: the same word is written that way by the exact lift
 * copySavedWorkPointerWords34To3CClearAllRecordActiveBytesRequestResource1AndAdvanceWorkByte1BmagicMagic11603_801EEC80
 * of emi/bmagic/magic116/03, which reads the record's words 0x34, 0x38 and 0x3C
 * through 0x74 exactly as this span does. What those three words mean stays
 * address-anchored here. */
extern u8 *D_801EC2E0; /* @source 0x801EC2E0 @kind unknown */

/* Effect transform of this overlay's own data run at 0x801EFA9C, the sixteen
 * bytes the ring pass 0x801EF040 reads as its origin point and its signed ring
 * radius. This span publishes the three origin words at +0x00, +0x04 and +0x08
 * from the saved work object's words 0x34, 0x38 and 0x3C, biasing the first by
 * 0x14000 and the third by 0x2C00000, and zeroes the ring radius at +0x10, the
 * burst radius at +0x12 and the fan angle at +0x14, which the handlers
 * 0x801EEE24, 0x801EEEC4 and 0x801EEF80 then ramp. +0x0C is written by nothing
 * in this overlay's image and stays unnamed. */
extern s32 T_801EFA9C; /* @source 0x801EFA9C @kind data */

extern s32 D_801EFAA0; /* @source 0x801EFAA0 @kind data */

extern s32 D_801EFAA4; /* @source 0x801EFAA4 @kind data */

extern s16 D_801EFAAC; /* @source 0x801EFAAC @kind data */

extern s16 D_801EFAAE; /* @source 0x801EFAAE @kind data */

extern s16 D_801EFAB0; /* @source 0x801EFAB0 @kind data */

/* Publishes the eight-slot table base 0x801EFAB4 into the cursor cell
 * 0x801EFAF4 and clears each slot leading state halfword. Already lifted exact
 * in this target as src/bof3/bmagic/stateSlotCursorStates.c
 * and bound by this target's own Splat listing, so this call takes no
 * arguments. */
void stateSlotCursorStates(void); /* @source 0x801EF6B0 */

/* Sound-cue dispatcher of the main executable at 0x8015DF18, outside this
 * overlay's image and bound through this target's generated
 * undefined_funcs_auto.txt; the exact siblings 0x801EEEC4 and 0x801EEDCC of
 * this target call it the same way with a cue word. This span passes cue 0x100
 * and keeps no result. */
void func_8015DF18(u32 arg0); /* @source 0x8015DF18 */

/* @source 0x801EECA4
 * @behavior Step handler 0 of this overlay, word 0 of the seven-word handler
 *           table at 0x801EEC04 (0x801EECA4, 0x801EED60, 0x801EEDCC,
 *           0x801EEE24, 0x801EEEC4, 0x801EEF80, 0x801EF00C) that the
 *           dispatcher 0x801EEC20 copies into its frame and indexes by unsigned
 *           work byte 1, so no jal and no word of this payload except that
 *           table word names this span. It takes the shared record through the
 *           pointer cell 0x801EC2E0, follows the record's word 0x74 to the
 *           saved work-object pointer, and publishes that object's words 0x34,
 *           0x38 and 0x3C as the effect origin at 0x801EFA9C, 0x801EFAA0 and
 *           0x801EFAA4, biasing the first by 0x14000 and the third by 0x2C00000.
 *           Each of those three stores re-reads the record word 0x74, so the
 *           walk is one pointer load per store rather than one saved pointer.
 *           It then zeroes the ring radius at 0x801EFAAC, the burst radius at
 *           0x801EFAAE and the fan angle at 0x801EFAB0, calls the slot reset
 *           0x801EF6B0 with no arguments so the eight effect slots 0x801EFAB4
 *           start idle, stores 0x3C into work byte 9 through the scratchpad
 *           cell 0x1F800044, re-reads that cell and increments work byte 1,
 *           which moves the dispatcher on to the next handler entry, and submits
 *           cue 0x100 through 0x8015DF18. Those are its only stores, it reads no
 *           other cell, it has no loop, and the 0x18-byte frame keeps $ra at
 *           $sp+0x10 across the two calls. It takes no arguments and returns
 *           nothing. The meaning of the record words 0x34/0x38/0x3C, of the
 *           0x14000 and 0x2C00000 biases and of the cue 0x100 stay
 *           address-anchored.
 * @status exact
 * @match 100.00
 * @residual none
 */
void publishEffectTransformFromSavedWorkObjectZeroRadiiResetEffectSlotsArmCountdownAdvanceWorkByte1AndSubmitCue100BmagicMagic15003_801EECA4(
    void) {
  /* The three origin stores are one statement group and the three halfword
   * clears are the next one, in the order the original's own stores show: the
   * clears sit after the 0x801EFAA4 word, not between the 0x801EFAA0 store and
   * it. Housed between those two words, the same five statements measured
   * 32/47 instructions at the same 188 bytes; moved after the 0x801EFAA4
   * store they measured 47/47. So the group order is the lever, not the
   * pointer spelling: a separately named work-pointer local, a reused record
   * local, a register-qualified record, a struct view of the record, and
   * pointer or word types for it all measured the same 32/47 in the misplaced
   * order. */
  u8 *record;

  record = D_801EC2E0;
  T_801EFA9C = *(s32 *)(*(u8 **)(record + 0x74) + 0x34) + 0x14000;
  D_801EFAA0 = *(s32 *)(*(u8 **)(record + 0x74) + 0x38);
  D_801EFAA4 = *(s32 *)(*(u8 **)(record + 0x74) + 0x3C) + 0x2C00000;
  D_801EFAAC = 0;
  D_801EFAAE = 0;
  D_801EFAB0 = 0;
  stateSlotCursorStates();
  g_battle_work[9] = 0x3C;
  g_battle_work[1]++;
  func_8015DF18(0x100);
}
