#include "bof3/bof3.h"

/* Front-side byte of the shared 0x80143F00 game-mode tuple; the overlay
 * request helpers only act while it reads zero. The original takes it with
 * `lbu`.
 * @source 0x80143F03 @kind unknown */
extern u8 D_80143F03;
/* Shared state-entry selector: dispatchIndexedStateHandler indexes the
 * D_801C84BC handler table with this byte, and the world overlays publish
 * their next state entry here. The original stores it with one `sb`.
 * @source 0x801448EB @kind unknown */
extern u8 D_801448EB;
/* Progress byte of the shared state-entry tuple 0x801448EB..0x801448ED, stored
 * with one `sb`.
 * @source 0x801448EC @kind unknown */
extern u8 D_801448EC;
/* Leading byte of the main-RAM game-mode record table 0x80145E98[], read here
 * with `lbu`. etc/game/00 declares the same table as GameModeRecord[] and
 * func_801C090C publishes this leading mode byte.
 * @source 0x80145E98 @kind data */
extern u8 D_80145E98;

void func_8015C088(void);

/* @source 0x801F4CC4
 * @behavior Overlay request that starts the shared state entry 0x23. It
 *   returns 0 while the front-side byte D_80143F03 is nonzero, while the first
 *   argument is not 0x248000, while the packed second argument's high half is
 *   outside 0x35..0x37, or while the game-mode byte D_80145E98 reads neither 6
 *   nor 7 nor 0. On a match it runs the shared front-end startup helper
 *   func_8015C088, stores 0x23 into the shared state-entry selector
 *   D_801448EB, clears the progress byte D_801448EC and returns 1. The 0x18-byte
 *   frame only keeps $ra across the call, and each of the two byte stores
 *   materialises the 0x8014 page in $at on its own.
 *
 * Name evidence: 0x801448EB is the state selector that etc/game/00 dispatches
 *   its D_801C84BC handler table through and that the world overlays set as
 *   their state entry; 0x801448EC is the progress byte of the same three-byte
 *   tuple; 0x80143F03 is the front-side byte the sibling overlay request
 *   helpers return on when it is nonzero; 0x80145E98 is the leading mode byte
 *   of the main-RAM game-mode record, declared u8 at its original `lbu` width
 *   in include/bof3/ui/game00_internal.h. The two arguments carry no evidenced
 *   role beyond a fixed 0x248000 command word and its packed 0x35..0x37 range,
 *   so the name keeps them structural.
 *
 * Shape: seeded from this target's original bytes alone, matching the sibling
 *   overlay requests startStateEntry0BProgress0AOr0WhenMapRangeAndScenarioGateWorld01Area04613_801F2CE4
 *   (0x801F2CE4) for the D_80143F03 gate, the `bne` command test, the
 *   `(u32)((packed >> 16) - base) >= 3u` range reject and the func_8015C088 +
 *   state-tuple writes, and selectAreaStateForMatchedArgumentsWorld04Area17913_801F3378
 *   (0x801F3378) for the mode-byte gate written as one positive block with the
 *   out-of-line reject after it. No byte-identical twin exists: the 148-byte
 *   span and its discriminating immediates 0x248000 and 0x35 occur only in this
 *   payload, so the clone lever is ineligible.
 * @status exact
 * @match 100.00
 * @residual none
 */
s32 startStateEntry23Progress0WhenPackedCommandAndModeByteWorld04Area17213_801F4CC4(
    s32 command, s32 packed) {
  u8 mode;

  if (D_80143F03 != 0) {
    return 0;
  }
  if (command != 0x248000) {
    return 0;
  }
  if ((u32)((packed >> 16) - 0x35) >= 3u) {
    return 0;
  }
  mode = D_80145E98;
  if ((u32)(mode - 6) < 2u || mode == 0) {
    func_8015C088();
    D_801448EB = 0x23;
    D_801448EC = 0;
    return 1;
  }
  return 0;
}
