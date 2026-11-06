#include "bof3/bof3.h"

extern u8 *D_1F800044;
void func_801F8C04(s32 x, s32 y, s16 radius, u8 center, u8 edge);
void func_8015DF18(u16 arg0);

/* @source 0x801F87A4
 * @behavior Entry 2 of this overlay's three-word work-byte-1 handler group at
 * 0x801FCFAC (word 12 of the 32-word code-pointer run at 0x801FCF84), called
 * with no arguments: it reads the work object published at scratchpad pointer
 * cell 0x1F800044 and calls the local drawing helper 0x801F8C04 with signed
 * halfwords at offsets 0x2E and 0x30, a radius of 0x3C with bit 0 of work byte
 * 9 set, center 0x80 and edge 0. It queues main-exe sound cue 0x208 through
 * func_8015DF18 when the reloaded work byte 9 reads 0xD7 (215), then decrements
 * work byte 9 and, when the stored countdown reaches zero, re-arms that byte
 * with 0x3C and advances work byte 1. Every pointer access re-reads the
 * scratchpad cell because the byte stores through the loaded pointer can alias
 * it; the 0x20-byte frame only keeps $ra across the two calls.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawTickWorkByte9QueueCue208WhenByte9Is215Reload60AndAdvanceScenarioScena0300_801F87A4(void) {
  u8 *draw_work;
  u8 countdown;

  draw_work = D_1F800044;
  func_801F8C04(FIELD_REF(s16, draw_work, 0x2E),
                FIELD_REF(s16, draw_work, 0x30),
                (draw_work[9] & 1) | 0x3C, 0x80, 0);
  if (D_1F800044[9] == 0xD7) {
    func_8015DF18(0x208);
  }
  countdown = D_1F800044[9] - 1;
  D_1F800044[9] = countdown;
  if (countdown == 0) {
    D_1F800044[9] = 0x3C;
    D_1F800044[1]++;
  }
}
