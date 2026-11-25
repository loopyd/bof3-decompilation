#include "bof3/bof3.h"

typedef void (*BossBoss05116Handler)(void);

/* Overlay entrypoints this installer publishes. The first is the overlay's
 * state switch: it tests the 0x2 bit of D_80146328 and, while it is set, stores
 * 0xF into D_80146875 and calls func_801DDED8, and otherwise calls
 * func_801DDF00. The second is an inert handler returning immediately. The
 * third returns a zero result, which the void slot type drops. */
void func_800C1A54(void);
void noopHandler86(void);
s32  zeroResult(void);

/* Shared hook slots. 0x801463A4 is the battle hook the shared battle step
 * submitBattlerRecordsAndStartPhaseStream loads and calls as
 * D_801463A4(); 0x801463A8 is the second slot of the same trio and
 * 0x801463AC the third, written by every boss overlay of this archive family. */
extern BossBoss05116Handler D_801463A4; /* @source 0x801463A4 @kind unknown */
extern BossBoss05116Handler D_801463A8; /* @source 0x801463A8 @kind unknown */
extern BossBoss05116Handler D_801463AC; /* @source 0x801463AC @kind unknown */

/* @source 0x800C1A14
 * @behavior Publishes this overlay's handler trio into the shared hook slots:
 * func_800C1A54 (state switch: tests the 0x2 bit of D_80146328 and, while it is
 * set, stores 0xF into D_80146875 and calls func_801DDED8, and otherwise calls
 * func_801DDF00) into 0x801463A4, the inert handler
 * noopHandler86 into 0x801463A8, and the zero-result
 * accessor zeroResult into 0x801463AC; the accessor's s32
 * result is dropped by the void slot type. It reads and writes no other state
 * and returns nothing. No in-payload caller: no jal immediate and no pointer
 * word of this payload names 0x800C1A14, so the main program reaches it through
 * this overlay's entry point. The 56-byte body holds the same statement order
 * and slot mapping as the sibling installer publishBattleHooksBossBoss05216_
 * 800C1B0C, whose words at the three stores differ only in their %lo immediate.
 * @status exact
 * @match 100.00
 * @residual none
 */
void publishBattleHooks11(void) {
  D_801463A4 = func_800C1A54;
  D_801463A8 = noopHandler86;
  D_801463AC = (BossBoss05116Handler)zeroResult;
}
