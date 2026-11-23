#include "bof3/bof3.h"

typedef void (*Boss00216Handler)(void);

/* Overlay entry points this installer publishes, taken only by address: no
 * other payload word names them and no jal reaches them. The third entry is the
 * overlay's third battle hook, which takes the hook mode byte and returns the
 * zero result the slot type drops. */
void func_800C1DD4(void);
void noopHandlerBossBoss00216_800C1E34(void);
u32 latchStartHookBit(
    u8 mode);

/* Shared hook slots. 0x801463A4/0x801463A8 are the battle hooks the battle
 * step loads and calls (jalr); 0x801463AC is the third slot of the same trio,
 * written by every boss overlay of this archive family. */
extern Boss00216Handler D_801463A4; /* @source 0x801463A4 @kind unknown */
extern Boss00216Handler D_801463A8; /* @source 0x801463A8 @kind unknown */
extern Boss00216Handler D_801463AC; /* @source 0x801463AC @kind unknown */

/* @source 0x800C1D50
 * @behavior Publishes this overlay's handler trio into the shared hook slots
 * in slot order: the overlay entry 0x800C1DD4 into 0x801463A4, the inert
 * handler noopHandlerBossBoss00216_800C1E34 into 0x801463A8 and the overlay
 * entry 0x800C1D88 into 0x801463AC, then returns to the caller. The sibling
 * installers at 0x800C1E3C and 0x800C1F08 of this overlay write the same three
 * slots in the same order and their bodies differ from this one only in the
 * three stored pointers. No payload word names this function and no jal
 * reaches it, so the main program enters it through the overlay entry point.
 * @status exact
 * @match 100.00
 * @residual none
 */
void publishBattleHooksBossBoss00216_800C1D50(void) {
  D_801463A4 = func_800C1DD4;
  D_801463A8 = noopHandlerBossBoss00216_800C1E34;
  D_801463AC =
      (Boss00216Handler)
          latchStartHookBit;
}
