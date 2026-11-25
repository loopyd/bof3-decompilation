#include "bof3/bof3.h"

typedef void (*Boss03816Handler)(void);

/* Overlay entrypoints this installer publishes. The first is this target's
 * battle-flag state switch at 0x800C1BBC, an `asm` boundary of this target: it
 * loads the shared flag byte D_80146328, tests bit 0x2, and while the bit is set
 * stores 5 into 0x80146875 and calls initModeTuple510 at 0x801DDED8, and when
 * the bit is clear it calls initModeTuple520 at 0x801DDF00. Its return type is
 * not proven; the void slot type only needs its address. The second is this
 * target's exact inert handler 0x800C1C08, and the third its exact zero-result
 * accessor 0x800C1BB4, which the void slot type drops. */
void func_800C1BBC(void);
void noopHandler8(void);
s32 zeroResult2(void);

/* Shared hook slots. 0x801463A4 and 0x801463A8 are the battle hooks the battle
 * step loads and calls: the battle overlay header
 * include/bof3/battle/battle03_internal.h binds both as handler cells, and its
 * own lifts call the loaded pointers, D_801463A4 from
 * submitBattlerRecordsAndStartPhaseStream and D_801463A8 from
 * startSelectionEmiStreamAndAdvanceState. 0x801463AC is the third slot of the
 * same trio, written by every boss overlay of this archive family. */
extern Boss03816Handler D_801463A4; /* @source 0x801463A4 @kind unknown */
extern Boss03816Handler D_801463A8; /* @source 0x801463A8 @kind unknown */
extern Boss03816Handler D_801463AC; /* @source 0x801463AC @kind unknown */

/* @source 0x800C1B7C
 * @behavior Publishes this overlay's handler trio into the shared battle hook
 * slots in slot order: the battle-flag state switch func_800C1BBC into
 * 0x801463A4, the inert handler noopHandler8 into
 * 0x801463A8, and the zero-result accessor zeroResult2
 * into 0x801463AC through the void slot type. The original materialises each
 * installed address in $v0 with lui/addiu and stores it through $at, and the
 * body has no frame, no saved register and no call of its own. No jal names
 * 0x800C1B7C and no word of this payload holds its address, so the overlay
 * entry reaches it. Same trio shape as the sibling installers
 * publishBattleHooksBossBoss03716_800C18FC and
 * publishBattleHooksBossBoss03316_800C1D70, which write the same three slots in
 * the same order; the bodies differ only in the three stored pointers.
 * @status exact
 * @match 100.00
 * @residual none
 */
void publishFlagHooks2(void) {
  D_801463A4 = func_800C1BBC;
  D_801463A8 = noopHandler8;
  D_801463AC = (Boss03816Handler)zeroResult2;
}
