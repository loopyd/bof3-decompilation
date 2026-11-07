#include "bof3/bof3.h"

typedef void (*Boss00413Handler)(void);

/* Overlay handlers this installer publishes. The first is the overlay's state
 * switch: it tests the 0x1 bit of D_80146328 and, while it is set, stores 7 into
 * D_80146255, sets the 0x80 bit of D_80146325, calls func_801DDF28 and stores
 * 0x32 into D_80146875, and otherwise increments D_80146875 before calling
 * func_801DDF28. The second is the battle setup routine: it calls
 * func_800AC844(0) and func_800AC844(1), func_800AC870 and func_800AC754,
 * publishes the 0x1F800044 work record through func_8014DD3C(0x5F) and
 * func_8014D6B8(1), and copies the D_801EB688/D_801EB68A and
 * D_801EB7A0/D_801EB7A2 pairs into that record. The third masks its argument
 * with 0xFF, switches on values below 7 through the 0x800C1804 table and
 * returns a zero result, which the void slot type drops. */
void func_800C1DE4(void);
void func_800C1E64(void);
s32  func_800C1C44(u8);

/* Shared hook slots. 0x801463A4/0x801463A8 are the battle hooks the battle
 * step loads and calls (jalr); 0x801463AC is the third slot of the same trio,
 * written by every boss overlay of this archive family. */
extern Boss00413Handler D_801463A4; /* @source 0x801463A4 @kind unknown */
extern Boss00413Handler D_801463A8; /* @source 0x801463A8 @kind unknown */
extern Boss00413Handler D_801463AC; /* @source 0x801463AC @kind unknown */

/* @source 0x800C1C0C
 * @behavior Publishes this overlay's handler trio into the shared hook slots:
 * func_800C1DE4 (state switch: tests the 0x1 bit of D_80146328 and, while it is
 * set, stores 7 into D_80146255, sets the 0x80 bit of D_80146325, calls
 * func_801DDF28 and stores 0x32 into D_80146875, and otherwise increments
 * D_80146875 before calling func_801DDF28) into 0x801463A4, the setup routine
 * func_800C1E64 (calls func_800AC844(0) and func_800AC844(1), func_800AC870 and
 * func_800AC754, publishes the 0x1F800044 work record through
 * func_8014DD3C(0x5F) and func_8014D6B8(1), and copies the
 * D_801EB688/D_801EB68A and D_801EB7A0/D_801EB7A2 pairs into that record) into
 * 0x801463A8, and the byte-indexed step dispatcher func_800C1C44 into
 * 0x801463AC; the dispatcher's s32 result is dropped by the void slot type.
 * Same trio shape as the sibling installers of this archive family, which write
 * the same three slots in the same order. No in-payload caller: no jal reaches
 * this function, so the main program reaches it through its overlay entry point.
 * @status exact
 * @match 100.00
 * @residual none
 */
void publishBattleHooksBossBoss00413_800C1C0C(void) {
  D_801463A4 = func_800C1DE4;
  D_801463A8 = func_800C1E64;
  D_801463AC = (Boss00413Handler)func_800C1C44;
}
