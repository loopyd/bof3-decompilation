#include "bof3/bof3.h"

/* Shared selector byte this handler gates on. The in-repo front-end
 * dispatchers of the same overlay family read it as one of the reset selectors
 * and clear it together with 0x801448EB and 0x801448EC. This body loads it with
 * `lbu`, so it stays u8. */
extern u8 D_801448ED; /* @source 0x801448ED @kind unknown */

/* Shared state-entry bytes the reset step of the same selectors clears. */
extern u8 D_801448EC; /* @source 0x801448EC @kind unknown */
extern u8 D_801448EB; /* @source 0x801448EB @kind unknown */

/* Shared small mode byte: this overlay's handlers read it as an index scaled by
 * four into the pointer table 0x80148210, as a bound-tested value below 8, and
 * as the 16-step geometry scale of the entity at 0x80146884. This body tests it
 * against 2, 6 and 8. */
extern u8 D_8014686B; /* @source 0x8014686B @kind unknown */

/* Shared overlay release helper of the resident executable at 0x8015C058. */
void func_8015C058(void);

/* Shared cue helper of the resident executable at 0x801BE1B0; the reviewed
 * front-end dispatchers pass one small cue id in the argument word. */
void func_801BE1B0(u32 arg0);

/* @source 0x801F4B60
 * @behavior Reads the shared selector byte 0x801448ED. While it reads 1, maps
 *   the shared mode byte 0x8014686B to a cue id (2 to 6, 6 to 7, 8 to 1, every
 *   other value to 8) and passes it to the shared cue helper 0x801BE1B0;
 *   otherwise runs that helper with 0 while the selector reads 0 and with 2
 *   while it reads 2. On every path it then runs the shared overlay release
 *   helper 0x8015C058 and clears the shared selector bytes 0x801448EB,
 *   0x801448EC and 0x801448ED. Takes no arguments and returns nothing.
 *
 * Name evidence: no payload word and no in-image `jal` names 0x801F4B60, so the
 *   caller stays unnamed and the body carries the whole role. The gated cell
 *   0x801448ED is one of the three reset selectors the family's reviewed
 *   dispatchers clear, and the argument words 0, 1, 2, 6, 7 and 8 are the cue
 *   ids those dispatchers hand to 0x801BE1B0.
 *
 * Shape: no payload of the 396 shipped binaries holds this 200-byte span - one
 *   scan of every payload found the single hit, in this target - and a second
 *   scan with each `jal` target masked found no sibling either, so neither clone
 *   nor a byte-identical twin applies. The span's own tail is shared word for
 *   word with emi/world04/area188/13 at offset 0x2154 (its mode-13 arm), which
 *   supplied the reviewed order of the release call and the three clears; this
 *   body writes its own selector and cue words from its own bytes. The switch
 *   arms are written in descending order (8, 6, 2) because the emitted arm
 *   blocks follow the source order: the ascending order measured 47/50 with the
 *   high arm pair swapped behind a `beq`, and the descending order measured
 *   50/50 at the original size.
 * @status exact
 * @match 100.00
 * @residual none
 */
void queueCueBySharedSelectorAndModeByteThenReleaseOverlayAndClearSharedSelectorsWorld01Area04813_801F4B60(void) {
  u8* selector;
  u32 cue;

  selector = &D_801448ED;
  if (*selector == 1) {
    switch (D_8014686B) {
    case 8:
      cue = 1;
      break;
    case 6:
      cue = 7;
      break;
    case 2:
      cue = 6;
      break;
    default:
      cue = 8;
      break;
    }
    func_801BE1B0(cue);
  } else {
    if (*selector == 0) {
      func_801BE1B0(0);
    }
    if (*selector == 2) {
      func_801BE1B0(2);
    }
  }
  func_8015C058();
  D_801448EB = 0;
  D_801448EC = 0;
  D_801448ED = 0;
}
