#include "bof3/bof3.h"

/* Published scratch work object: the original materialises this cell with a
 * lui plus an offset load, so the cell is its own fixed-address symbol. */
extern u8* D_1F800044; /* @source 0x1F800044 @kind data */

/* Ten-byte effect-record array of the shared main executable at 0x800EA200.
 * Byte 0 of a record is the blink flag this body reads, complements and
 * restores, byte 1 its life counter, halfword 2 the packed 5-5-5 colour the draw
 * expands, halfwords 4 and 6 the position pair the signed velocity bytes 8 and 9
 * are subtracted from. The record base is spelled as its integer address because
 * the original materialises 0x800EA200 and its own derived 0x800EA202 base with
 * `lui` plus `ori` pairs and the measured symbol-address form emits `lui` plus
 * `addiu` instead. The neighbouring overlay world02/area098/13 walks the same
 * layout from 0x800E6800 and 0x800E6802 with the same offsets, stride and colour
 * expansion through the same two shared calls, so that pair is shape evidence
 * and not a byte-identical twin. */

/* Primitive cursor cell of the shared main executable. */
extern u8* g_PrimCursor; /* @source 0x8014598C @kind bss */

/* Scenario progress byte of the shared main executable. */
extern u8 g_ScenarioProgress;

/* Active effect-record count of this payload's data blob: it sits inside the
 * Splat `textbin` run at 0x801D3918, so the generated undefined_syms_auto.txt
 * row already names it and no map row is owed. */
extern u16 D_801D40A8; /* @source 0x801D40A8 @kind data */

/* Primitive preparation helper of the shared main executable at 0x8017AA30: the
 * sibling lifts of the bmagic and world overlay families declare the same word
 * argument and pass the cursor taken from g_PrimCursor. */
void func_8017AA30(void* arg0);

/* Primitive submit helper of the shared main executable at 0x8014E5A0: it takes
 * the OT slot and the primitive size as its two byte arguments, and the exact
 * lift drawRedGradientBandCenteredOnWorkEntryHalfwordWithFrameParityHalfHeightScenarioSce15ef200_801D1414
 * declares the same pair. */
void func_8014E5A0(u8 arg0, u8 arg1);

/* @source 0x801D34F0
 * @behavior Blinking effect-record pass of this overlay: it first sets bit 6 of
 *           the flag byte behind the pointer word at +0x4C of the scratchpad
 *           work object, then walks the ten-byte effect records of the shared
 *           array at 0x800EA200 up to this payload's own count halfword at
 *           0x801D40A8. A record whose blink flag byte is set draws: while its
 *           life byte is nonzero it subtracts the signed velocity bytes 8 and 9
 *           from the position halfwords 4 and 6 and decrements that life, then
 *           takes the cursor cell at 0x8014598C, prepares the primitive through
 *           func_8017AA30, stores the record position into cursor halfwords 8
 *           and 10, expands the packed colour halfword 2 into cursor bytes 4, 5
 *           and 6 by shifting each five-bit channel left by three (mask 0x1F,
 *           0x3E0 and 0x7C00) and submits the 0xC-byte primitive under OT slot 2
 *           through func_8014E5A0. Every record, drawn or not, has its blink flag
 *           complemented, so a record is drawn on alternate passes. The body then
 *           ticks work countdown byte 9 down by one and, when it reaches zero,
 *           stores progress 0x2B into the shared byte 0x80146864 and advances
 *           work byte 1 by one. Every field and the cursor are reloaded after the
 *           two calls because those stores may alias them, and the count halfword
 *           is read again for each pass; the work cell is loaded before the frame
 *           is created and re-read for the countdown and for its byte 1.
 *           The red channel reads the colour as a byte and the green and blue
 *           channels as halfwords, which is the three-load form the original
 *           shows.
 *           The position, speed and life updates of one record are the only
 *           surviving residual; every other instruction of the span matches.
 * @status partial
 * @match 90.65
 * @residual The per-record update block. The original hoists its five loads
 *           (speed 8, speed 9, life 1, position 4, position 6) above every
 *           arithmetic instruction and keeps the three stores (position 4,
 *           position 6, life 1) together at the end, while this candidate
 *           measures the same fifteen instructions in statement order with the
 *           signs extended where each subtraction needs them: 97 of 107
 *           instructions and all 428 bytes. Spelling those five values as
 *           locals does hoist the loads, but the life load then folds into the
 *           guard load and the frame shortens by eight bytes to 420; both the
 *           delayed-branch fill and that fold are what keep the two forms apart.
 *
 * Matching note: the walk over one record base was measured first as an indexed
 * access over 0x800EA200, which multiplied the counter by ten on every pass,
 * then as two separate record symbols, which grew the frame by a sixth saved
 * register. The single record base advanced by ten is what reproduces the
 * original two-base split at 0x800EA200 and 0x800EA202 with the -1, 0, 2, 4, 6
 * and 7 offsets and the 0x28 frame.
 */
void drawBlinkingTintedEffectRecordsAdvanceByVelocityTickCountdown9ThenSetProgress43AndAdvanceWorkByte1ScenarioSce15ef100_801D34F0(void) {
  u8* record;
  u8* prim;
  u16 i;

  *(u8*)*(u32*)(D_1F800044 + 0x4C) |= 0x40;
  record = PSX_PTR(u8, 0x800EA200);
  for (i = 0; i < D_801D40A8; record += 0xA, i++) {
    if (record[0] != 0) {
      if (record[1] != 0) {
        *(u16*)(record + 4) = *(u16*)(record + 4) - (s8)record[8];
        record[1] = record[1] - 1;
        *(u16*)(record + 6) = *(u16*)(record + 6) - (s8)record[9];
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
    g_ScenarioProgress = 0x2B;
    D_1F800044[1]++;
  }
}
