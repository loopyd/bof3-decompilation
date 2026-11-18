#include "bof3/bof3.h"

/* The scratchpad work-object pointer cell every overlay shares, and the two
 * gate cells this overlay's step handlers test: the sibling handler 0x800C3038
 * reads the same two gates and moves the same work byte in the other
 * direction. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */
extern u8 D_80148603;     /* @source 0x80148603 @kind unknown */
extern u8 D_80146329;     /* @source 0x80146329 @kind unknown */

/* Shared battle state: the counter 0x801463D0 that the sibling hook dispatcher
 * 0x800C2760 compares with 0x15 in its mode-2 arm and that this handler prints
 * as 0x15 minus itself, and the mode byte 0x801462E0 the same dispatcher reads
 * for its mode-3 arm. */
extern u32 D_801463D0; /* @source 0x801463D0 @kind unknown */
extern u8 D_801462E0;  /* @source 0x801462E0 @kind unknown */

/* The shared text buffer the game formats names and counts into. */
extern u8 D_801490F8[]; /* @source 0x801490F8 @kind data */

/* In-payload text runs. 0x800C184C is the four-byte format "%2d" in the
 * leading textbin row 0x800C1804; 0x800C32A8 is the run "turns" 0xFF "left",
 * the word after the five-entry handler run 0x800C3294, inside the trailing
 * textbin row 0x800C3174. */
extern u8 D_800C184C[]; /* @source 0x800C184C @kind data */
extern u8 D_800C32A8[]; /* @source 0x800C32A8 @kind data */

extern int sprintf(char* buffer, const char* format, ...);

/* Drawers of the main executable, outside this payload, bound at their own
 * addresses: the panel strip drawer 0x801D8270 at (arg0, arg1), the text run
 * drawer 0x8014FF0C at (arg0, arg1) taking an offset and the run pointer, the
 * run drawer 0x8014F800 taking two more arguments with the run pointer on the
 * stack, and the shared helper 0x801E5988. */
extern void func_801D8270(s16 arg0, s16 arg1);
extern void func_8014FF0C(s16 arg0, s16 arg1, s32 arg2, const void* arg3);
extern void func_8014F800(s16 arg0, s16 arg1, s32 arg2, u32 arg3, u32 arg4);
extern void func_801E5988(void);

/* @source 0x800C3088
 * @behavior Fifth word of the five-entry handler run at 0x800C3294, the words
 *           0x800C2128, 0x800C2160, 0x800C217C, 0x800C3038 and 0x800C3088,
 *           which the sibling dispatcher 0x800C2FF4 reaches as its second
 *           entry, so this handler runs when work-record byte 1 holds one.
 *           It draws the panel strip at (0x6B, 0x10) through the EXE drawer
 *           0x801D8270, formats 0x15 minus the shared counter 0x801463D0 with
 *           the overlay format "%2d" at 0x800C184C into the shared text buffer
 *           0x801490F8, submits that buffer at (0x70, 0x12) from offset 4
 *           through 0x8014FF0C, and submits the overlay run 0x800C32A8, the
 *           text "turns" 0xFF "left", at (0x80, 0x12) with a zero third
 *           argument and 0xA as the fourth through 0x8014F800, so the panel
 *           shows the turns left of the counter 0x801463D0. It then reads the
 *           same two gate cells the sibling handler 0x800C3038 tests and, when
 *           either of them is set (the global byte 0x80148603 non-zero or bit
 *           0x2 of 0x80146329 set), loads the scratchpad work-object pointer
 *           cell 0x1F800044 once and decrements the work object byte at offset
 *           1 at byte width, moving the dispatch state byte from one back to
 *           zero. Last, while the shared mode byte 0x801462E0 reads 5, it
 *           calls the shared helper 0x801E5988. It takes no arguments and
 *           returns nothing; the 0x20-byte frame keeps $ra at 0x1C($sp), $s0
 *           at 0x18($sp) across the four calls, holding the text buffer, and
 *           the fifth argument of the last call at 0x10($sp). The run word at
 *           0x800C32A4 is the only in-image word equal to this address and no
 *           jal in the payload reaches it, so the run is the caller that names
 *           this handler.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawTurnsLeftPanelAndStepDispatchStateByte1BackWhenGateSetBossBoss02513_800C3088(void) {
  char* buffer;

  func_801D8270(0x6b, 0x10);
  buffer = (char*)D_801490F8;
  sprintf(buffer, (const char*)D_800C184C, 0x15 - D_801463D0);
  func_8014FF0C(0x70, 0x12, 4, buffer);
  func_8014F800(0x80, 0x12, 0, 0xa, (u32)D_800C32A8);
  if (D_80148603 != 0 || (D_80146329 & 0x02) != 0) {
    g_battle_work[1]--;
  }
  if (D_801462E0 == 5) {
    func_801E5988();
  }
}
