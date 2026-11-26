#include "base/types.h"

extern void renderOffsetTilePair(u32 mode); /* @source 0x801F07B0 @kind function */
extern void renderPixelGlyphScreen(u8 index);
extern u8* g_battle_work; /* @source 0x1F800044 @kind data */
extern u16 D_80145AA8; /* @source 0x80145AA8 @kind unknown */
extern u8 D_801F0A98[]; /* @source 0x801F0A98 @kind data */

/* @source 0x801EEF4C
 * @behavior Calls overlay helper 0x801F07B0 with 4, draws work byte 0xB as a
 * grey 12-pixel glyph through the work screen pair, then reads the shared
 * pressed-pad-state halfword D_80145AA8. When its low-byte d-pad nibble is
 * set, reloads the work pointer through the scratchpad cell at 0x1F800044 and
 * compares the whole halfword with the byte table at 0x801F0A98 indexed by
 * work byte 0xB; that table's first four bytes are the d-pad masks 0x20, 0x40,
 * 0x10 and 0x80. On a match it rearms work byte 9 to 6, increments work byte
 * 0xA and advances work byte 1 to 3 while the incremented byte stays below
 * 0x20; when it reaches 0x20, on a mismatch, and on a timeout it rearms byte 9
 * to 6 and advances work byte 1 to 5. A timeout is the d-pad nibble clear on
 * this frame and the decremented unsigned work byte 9 reaching zero. The body
 * is reached through the byte-1 handler pointer run at 0x801F0AD4, so the run
 * carries no prototype and the roles stay structural. Takes no arguments and
 * returns nothing. Helper 0x801F07B0's mode role remains unknown.
 * @status exact
 * @match 100.00
 * @residual none
 */
void renderGlyphMatchDpad(void) {
  u8* work;
  u16 pad;

  renderOffsetTilePair(4);
  renderPixelGlyphScreen(g_battle_work[0xB]);
  pad = D_80145AA8;
  if (pad & 0xF0) {
    work = g_battle_work;
    if (pad == D_801F0A98[work[0xB]]) {
      work[9] = 6;
      work = g_battle_work;
      if (++work[0xA] < 0x20) {
        work = g_battle_work;
        work[1] = 3;
        return;
      }
    } else {
      work[9] = 6;
    }
  } else {
    work = g_battle_work;
    if (--work[9] != 0) {
      return;
    }
    work = g_battle_work;
    work[9] = 6;
  }
  work = g_battle_work;
  work[1] = 5;
}
