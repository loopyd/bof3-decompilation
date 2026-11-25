#include "bof3/bof3.h"

typedef void (*Boss02816Handler)(void);

/* Overlay entrypoints this installer publishes. The first is the overlay's
 * state switch at 0x800C1B80; the second is the thin wrapper that calls the
 * main-program helper at 0x800AC934 with argument zero; the third returns a
 * zero result, which the void slot type drops. */
void setupFlagBitTwo(
    void);
void invokeZeroArgOverlay(void);
s32  zeroResult33(void);

/* Shared hook slots. 0x801463A4/0x801463A8 are the battle hooks the battle
 * step loads and calls (jalr); 0x801463AC is the third slot of the same trio,
 * written by every boss overlay of this archive family. */
extern Boss02816Handler D_801463A4; /* @source 0x801463A4 @kind unknown */
extern Boss02816Handler D_801463A8; /* @source 0x801463A8 @kind unknown */
extern Boss02816Handler D_801463AC; /* @source 0x801463AC @kind unknown */

/* @source 0x800C1B40
 * @behavior Publishes this overlay's handler trio into the shared hook slots:
 * the battle-flag state switch 0x800C1B80 (writes scenario progress 0x1E and
 * seeds the battle mode tuple with 5, 1, 0 when flag bit 0x2 is set) into
 * 0x801463A4, the thin wrapper
 * invokeZeroArgOverlay (calls 0x800AC934 with argument zero)
 * into 0x801463A8, and the zero-result accessor
 * zeroResult33 into 0x801463AC; the accessor's s32 result
 * is dropped by the void slot type. Same trio shape as the sibling installers
 * of this archive family, which store the same three slots in the same order.
 * No in-payload caller: neither this function's address nor the three slot
 * addresses appear in any other word of this payload, and no jal reaches it,
 * so the main program reaches it through its overlay entry point.
 * @status exact
 * @match 100.00
 * @residual none
 */
void publishBattleHooks20(void) {
  D_801463A4 = setupFlagBitTwo;
  D_801463A8 = invokeZeroArgOverlay;
  D_801463AC = (Boss02816Handler)zeroResult33;
}
