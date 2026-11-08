#include "bof3/bof3.h"

typedef void (*Boss00413Handler)(void);

/* Overlay handlers this installer publishes. The first is the overlay state
 * switch: it tests the 0x1 bit of D_80146328 and, while it is set, stores 8
 * into D_80146255, sets the 0x80 bit of D_80146325, calls func_801DDF28 and
 * stores 0x32 into D_80146875, and otherwise increments D_80146875 before
 * calling func_801DDF28. The second is the battle setup routine: it tests the
 * same bit and calls func_800AC844(2) and func_800AC844(3) while it is set,
 * and on the clear path runs func_800AC870(2, D_801EB630, 1) and
 * func_800AC754(2), publishes and clears the 0x1F800044 work record through
 * func_8014DD3C(0x5F) and func_8014D6B8(1), and copies the
 * D_801EB688/D_801EB68A and D_801EB7A0/D_801EB7A2 pairs into that record. The
 * third masks its argument with 0xFF, switches on values below 7 through the
 * 0x800C1824 table and returns a zero result, which the void slot type drops. */
void func_800C2188(void);
void func_800C2208(void);
s32  func_800C1FE8(u8);

/* Shared hook slots. 0x801463A4/0x801463A8 are the battle hooks the battle
 * step loads and calls (jalr); 0x801463AC is the third slot of the same trio,
 * written by every boss overlay of this archive family. */
extern Boss00413Handler D_801463A4; /* @source 0x801463A4 @kind unknown */
extern Boss00413Handler D_801463A8; /* @source 0x801463A8 @kind unknown */
extern Boss00413Handler D_801463AC; /* @source 0x801463AC @kind unknown */

/* @source 0x800C1FB0
 * @behavior Publishes this overlay second handler trio into the shared hook
 * slots: the state switch func_800C2188 (tests the 0x1 bit of D_80146328 and,
 * while it is set, stores 8 into D_80146255, sets the 0x80 bit of D_80146325,
 * calls func_801DDF28 and stores 0x32 into D_80146875, and otherwise
 * increments D_80146875 before calling func_801DDF28) into 0x801463A4, the
 * setup routine func_800C2208 (calls func_800AC844(2) and func_800AC844(3) on
 * the set path, and on the clear path func_800AC870(2, D_801EB630, 1),
 * func_800AC754(2), the 0x1F800044 work record publication through
 * func_8014DD3C(0x5F) and func_8014D6B8(1), and the
 * D_801EB688/D_801EB68A and D_801EB7A0/D_801EB7A2 record copies) into
 * 0x801463A8, and the byte-masked step dispatcher func_800C1FE8 into
 * 0x801463AC; that dispatcher returns a zero result, which the void slot type
 * drops. Same trio shape as the overlay installers of this archive family,
 * which write the same three slots in the same order. No in-payload caller: no
 * jal and no pointer word in the payload names this function, so the main
 * program reaches it through its overlay entry point.
 * @status exact
 * @match 100.00
 * @residual none
 */
void publishBattleHooksBossBoss00413_800C1FB0(void) {
  D_801463A4 = func_800C2188;
  D_801463A8 = func_800C2208;
  D_801463AC = (Boss00413Handler)func_800C1FE8;
}
