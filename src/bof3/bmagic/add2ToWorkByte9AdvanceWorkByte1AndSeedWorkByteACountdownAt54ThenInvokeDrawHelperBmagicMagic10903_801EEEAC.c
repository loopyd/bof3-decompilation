#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell shared by the battle and bmagic overlays.
 * The original re-materialises the cell (lui $vN,0x1F80 / lw $vN,68($vN)) at
 * each read, so it is named here rather than reached through the constant-address
 * accessor, which folds into a base that stays live across the trailing call.
 * The read-modify-write at entry keeps one load, and the two arm stores plus the
 * call each re-read it because a store through the loaded pointer may alias the
 * cell. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Draw routine of this overlay at 0x801EF0F8, still unlifted in its own asm
 * subsegment, which owns the symbol. It multiplies its argument into the sprite
 * channel offsets it emits (mult of the argument, then sra 12), so this call
 * passes one signed word. */
void func_801EF0F8(s32 arg0);

/* @source 0x801EEEAC
 * @behavior Step 1 of this overlay's four-word step run at 0x801EEC04, whose
 *   words are 0x801EEC80, 0x801EEEAC, 0x801EEF30 and 0x801EF008: the per-frame
 *   dispatcher at 0x801EEC14 copies those words into its frame and calls the
 *   one selected by unsigned work byte 0x01, so work byte 0x01 holds 1 while
 *   this body runs and no jal in the image targets this address. It adds two to
 *   the scratchpad work object's byte 0x09 modulo 256 through a single read of
 *   the pointer cell at 0x1F800044 (lbu / addiu 2 / sb) and compares the
 *   truncated value it just stored (andi 0xFF of the stored register, no second
 *   load) with 0x54. Only when that byte reaches 0x54 does it re-read the cell
 *   and increment work byte 0x01, advancing the dispatcher to handler 2 at
 *   0x801EEF30, then re-read the cell again and seed work byte 0x0A with 0x40 -
 *   the countdown that handler decrements once per frame before it leaves the
 *   step and that keeps the object on this run's second entry for about 0x40
 *   frames. Both paths then re-read the cell and call this overlay's draw
 *   routine at 0x801EF0F8 with the object's word at 0x64, the value handler 2
 *   ramps by 8 within -0x80..0x80 and the draw routine scales into its
 *   channels. The pointer load precedes frame creation; the 0x18-byte frame
 *   only keeps $ra at 0x10 across that call. It takes no arguments, returns
 *   nothing and reads or writes no other state.
 * @status exact
 * @match 100.00
 * @residual none
 */
void add2ToWorkByte9AdvanceWorkByte1AndSeedWorkByteACountdownAt54ThenInvokeDrawHelperBmagicMagic10903_801EEEAC(
    void) {
  u8 *work;
  u8 *drawWork;
  u8 countdown;

  work = g_battle_work;
  countdown = work[9] + 2;
  work[9] = countdown;
  if (countdown == 0x54) {
    g_battle_work[1]++;
    g_battle_work[0xA] = 0x40;
  }
  drawWork = g_battle_work;
  func_801EF0F8(*(s32 *)(drawWork + 0x64));
}
