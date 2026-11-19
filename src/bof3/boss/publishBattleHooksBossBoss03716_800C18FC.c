#include "bof3/bof3.h"

typedef void (*Boss03716Handler)(void);

/* Overlay entrypoints this installer publishes. The first is the battle-flag
 * state switch at 0x800C193C, an `asm` boundary of this target: it loads the
 * shared flag byte D_80146328, tests bit 0x2, and either stores 0x1C into
 * 0x80146875 and calls func_801DDED8, or calls func_801DDF00. Its return type is
 * not proven; the void slot type only needs its address. The second is this
 * target's exact inert handler 0x800C1988, and the third its exact zero-result
 * accessor 0x800C1934, which the void slot type drops. */
void func_800C193C(void);
void noopHandlerBoss037_800C1988(void);
s32 zeroResultBoss037_800C1934(void);

/* Shared hook slots. 0x801463A4/0x801463A8 are the battle hooks the battle step
 * loads and calls (jalr); 0x801463AC is the third slot of the same trio, written
 * by every boss overlay of this archive family. */
extern Boss03716Handler D_801463A4; /* @source 0x801463A4 @kind unknown */
extern Boss03716Handler D_801463A8; /* @source 0x801463A8 @kind unknown */
extern Boss03716Handler D_801463AC; /* @source 0x801463AC @kind unknown */

/* @source 0x800C18FC
 * @behavior Publishes this overlay's handler trio into the shared battle hook
 * slots in slot order: the battle-flag state switch func_800C193C into
 * 0x801463A4, the inert handler noopHandlerBoss037_800C1988 into 0x801463A8,
 * and the zero-result accessor zeroResultBoss037_800C1934 into 0x801463AC
 * through the void slot type. The original materialises each installed address
 * in $v0 with lui/addiu and stores it through $at, and the body has no frame,
 * no saved register and no call of its own. No jal names 0x800C18FC and no word
 * of this payload holds its address, so the overlay entry reaches it. The trio
 * itself is shared: the consumers the exact sibling installers established are
 * the battle step, which loads 0x801463A4 and 0x801463A8 and calls the loaded
 * pointer with jalr, and calls the third slot 0x801463AC with an argument. Same
 * trio shape as the exact siblings publishBattleHooksBossBoss00216_800C1E3C and
 * publishBattleHooksBossBoss00716_800C1A0C, which write the same three slots in
 * the same order; the bodies differ only in the three stored pointers.
 * @status exact
 * @match 100.00
 * @residual none
 */
void publishBattleHooksBossBoss03716_800C18FC(void) {
  D_801463A4 = func_800C193C;
  D_801463A8 = noopHandlerBoss037_800C1988;
  D_801463AC = (Boss03716Handler)zeroResultBoss037_800C1934;
}
