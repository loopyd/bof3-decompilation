#include "bof3/bof3.h"

/* Published scratch work object: the original materialises this cell with a
 * lui plus offset load, so the cell is its own fixed-address symbol. */
extern u8* D_1F800044; /* @source 0x1F800044 @kind data */

/* Ten-byte effect-record table of the shared main executable at 0x800EA200.
 * Byte 0 of a record is the blink flag this body reads, complements and
 * restores, byte 1 its life counter, halfword 2 the packed 5-5-5 colour the draw
 * expands, halfwords 4 and 6 the position pair the signed velocity bytes 8 and 9
 * are added to. The record base is spelled as its integer address because the
 * original materialises 0x800EA200 and its own derived 0x800EA202 base with
 * `lui` plus `ori` pairs and the measured symbol-address form emits `lui` plus
 * `addiu` instead. The neighbouring record passes at 0x801D26BC (still asm) and
 * 0x801D34F0 (lifted partial) walk the same table with the same stride, so that
 * family is shape evidence rather than a byte-identical twin of this span. */

/* Primitive cursor cell of the shared main executable. */
extern u8* g_PrimCursor; /* @source 0x8014598C @kind bss */

/* Active effect-record count of this payload's data blob: it sits inside the
 * Splat `textbin` run at 0x801D3918, so the generated undefined_syms_auto.txt
 * row already names it and no map row is owed. */
extern u16 D_801D40A8; /* @source 0x801D40A8 @kind data */

/* Primitive preparation helper of the shared main executable at 0x8017AA30: the
 * sibling lifts of this overlay family declare the same word argument and pass
 * the cursor taken from g_PrimCursor. */
void func_8017AA30(void* arg0);

/* Primitive submit helper of the shared main executable at 0x8014E5A0: it takes
 * the OT slot and the primitive size as its two byte arguments, and the 0x801D34F0
 * lift above declared the same pair. */
void func_8014E5A0(u8 arg0, u8 arg1);

/* @source 0x801D2934
 * @behavior Entry 4 of the ten-word overlay step-handler code-pointer run at
 *           0x801D3954 and of the eleven-word run at 0x801D397C: it first sets
 *           bit 6 of the flag byte behind the pointer word at +0x4C of the
 *           scratchpad work object, then walks the ten-byte effect records of
 *           the shared table at 0x800EA200 up to this payload count halfword at
 *           0x801D40A8. A record whose blink flag byte reads nonzero is
 *           handled through the scratchpad work byte at +0x6: while that byte
 *           reads zero the record adds its signed velocity bytes +0x8 and +0x9
 *           into its position halfwords +0x4 and +0x6 and decrements its life
 *           byte +0x1 only while that life byte reads nonzero, and while that
 *           byte reads nonzero the record decrements the life byte in place and
 *           adds the velocity only once the life byte has reached zero. It then
 *           takes the cursor cell at 0x8014598C, prepares the 0xC-byte primitive
 *           through func_8017AA30, stores the record position into cursor
 *           halfwords 8 and 10, expands the packed colour halfword +0x2 into
 *           cursor bytes 4, 5 and 6 with the masks 0x1F, 0x3E0 and 0x7C00 and
 *           submits it under OT slot 2 through func_8014E5A0. Every record,
 *           drawn or not, has its blink flag complemented, so a record is drawn
 *           on alternate passes. The body then ticks work countdown byte +0x9
 *           down by one and, when it reaches zero, advances work byte +0x1 by
 *           one; it stores no scenario progress. Every field and the cursor are
 *           reloaded after the two calls because those stores may alias them,
 *           and the count halfword is read again for each pass.
 * @status partial
 * @match 91.67
 * @residual The per-record update block of the work-byte-6-clear arm, 11 of the
 *           132 checked instructions. The original hoists its five loads (the
 *           velocity bytes 8 and 9, the life byte 1, the position halfwords 4
 *           and 6) above every arithmetic instruction and leaves the three
 *           stores to the end, which also fixes the register assignment it uses
 *           for them. This candidate measures the same fifteen instructions in
 *           statement order: it hoists the first three loads, then extends, adds
 *           and stores the first position before loading the second position and
 *           the life byte. Eight of the seventeen clean-C variants measured
 *           reproduced this build exactly, 528 bytes with the first difference at
 *           +0x0090 (velocity values bound to locals, position values bound to
 *           locals with the stores last, compound assignment, a comma-grouped
 *           update statement, a halfword-pair view, a byte view derived from that
 *           pair, and typed record members). The other nine changed it: every
 *           variant that put the decrement first or between the two positions, or
 *           bound the new position values to locals, measured 480 to 524 bytes
 *           with the first difference at +0x0044, and the byte operand written
 *           first stayed at 528 bytes but lost nine matched instructions. The
 *           residual is the post-reload scheduling and allocation of one
 *           straight-line block, not a statement order this source has yet to
 *           reproduce.
 *
 * Matching note: the walk over one record base is spelled as one pointer advanced
 * by ten, which reproduced the original split at 0x800EA200 and 0x800EA202 with
 * the -1, 0, 2, 4, 6 and 7 offsets and the 0x28 frame in this candidate's live
 * measurement; the 0x801D34F0 lift above measured that same split with the same
 * spelling. */
void drawBlinkingTintedEffectRecordsAdvanceByVelocityBeforeOrAfterLifeExpiryByWorkByte6TickCountdown9ThenAdvanceWorkByte1ScenarioSce15ef100_801D2934(void) {
  u8* record;
  u8* prim;
  u16 i;

  *(u8*)*(u32*)(D_1F800044 + 0x4C) |= 0x40;
  record = PSX_PTR(u8, 0x800EA200);
  for (i = 0; i < D_801D40A8; record += 0xA, i++) {
    if (record[0] != 0) {
      if (D_1F800044[6] == 0) {
        if (record[1] != 0) {
          *(u16*)(record + 4) = *(u16*)(record + 4) + (s8)record[8];
          *(u16*)(record + 6) = *(u16*)(record + 6) + (s8)record[9];
          record[1] = record[1] - 1;
        }
      } else if (record[1] != 0) {
        record[1] = record[1] - 1;
      } else {
        *(u16*)(record + 4) = *(u16*)(record + 4) + (s8)record[8];
        *(u16*)(record + 6) = *(u16*)(record + 6) + (s8)record[9];
      }
      prim = g_PrimCursor;
      func_8017AA30(prim);
      *(u16*)(prim + 8) = *(u16*)(record + 4);
      *(u16*)(prim + 0xA) = *(u16*)(record + 6);
      prim[4] = (record[2] & 0x1F) << 3;
      prim[5] = (*(u16*)(record + 2) & 0x3E0) >> 2;
      prim[6] = (*(u16*)(record + 2) & 0x7C00) >> 7;
      func_8014E5A0(2, 0xC);
    }
    record[0] = record[0] ^ 1;
  }
  if (--D_1F800044[9] == 0) {
    D_1F800044[1]++;
  }
}
