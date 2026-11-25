#include "bof3/bof3.h"

typedef void (*Boss00716Handler)(void);

/* Overlay entrypoints this installer publishes. The first is the overlay's
 * battle-flag state switch: it tests the 0x1 bit of the shared flag byte
 * 0x80146328, stores the mode tuple 6/6/0x32 and runs initModeTuple530 while it
 * is set, and otherwise stores 2 into 0x80146255, clears the effect-bank byte
 * 0x8014832E, raises bit 0x8 of the flag byte, advances 0x80146875 and stores 4
 * into 0x80145029. The second calls func_800AC844 with zero and then with one.
 * The third is this payload's other handler entry at 0x800C1A44, still an `asm`
 * boundary of this target. */
void setupBitEffectBank(
    void);
void callArgZeroOne(void);
void func_800C1A44(void);

/* Shared hook slots. 0x801463A4/0x801463A8 are the battle hooks the battle
 * step loads and calls (jalr); 0x801463AC is the third slot of the same trio,
 * written by every boss overlay of this archive family. */
extern Boss00716Handler D_801463A4; /* @source 0x801463A4 @kind unknown */
extern Boss00716Handler D_801463A8; /* @source 0x801463A8 @kind unknown */
extern Boss00716Handler D_801463AC; /* @source 0x801463AC @kind unknown */

/* @source 0x800C1A0C
 * @behavior Publishes this overlay's handler trio into the shared battle hook
 * slots: setupBitEffectBank
 * (the battle-flag state switch) into 0x801463A4,
 * callArgZeroOne (calls func_800AC844 with 0
 * and then with 1) into 0x801463A8, and func_800C1A44, this payload's 0x270-byte
 * handler entry, into 0x801463AC. All three slots take a void handler; the
 * original materialises each installed address in $v0 and stores it through
 * $at, and the body has no frame and no call of its own. No jal names
 * 0x800C1A0C and no other word of this payload holds its address, so the
 * overlay entry reaches it. BATTLE.EMI#03 is the consumer: it loads 0x801463A4
 * at 0x600C and 0x801463A8 at 0x6650 and calls the loaded pointer with jalr, and
 * calls the third slot 0x801463AC with argument 3 at 0x594, 6 at 0xB84 and 1 at
 * 0x23D8. Same trio shape as the exact siblings
 * publishBattleHooksBossBoss05216_800C1B0C and
 * publishBattleHooksBossBoss02416_800C1AB0, which write the same three slots in
 * the same order; the bodies differ only in the three stored pointers.
 * @status exact
 * @match 100.00
 * @residual none
 */
void publishBattleHooks13(void) {
  D_801463A4 =
      setupBitEffectBank;
  D_801463A8 = callArgZeroOne;
  D_801463AC = func_800C1A44;
}
