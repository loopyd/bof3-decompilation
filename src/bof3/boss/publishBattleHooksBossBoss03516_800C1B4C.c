#include "bof3/bof3.h"

typedef void (*Boss03516Handler)(void);

/* Overlay entrypoints this installer publishes. The first is the overlay's
 * state switch: it tests the 0x2 bit of D_80146328 and, while it is set, stores
 * 0x15 into the byte at D_80146875 and calls func_801DDED8, and otherwise calls
 * func_801DDF00. The second reloads the halfword at D_801462E8, sets its 0x2
 * bit and calls func_8014ECAC with 4. The third is the overlay's indexed
 * handler dispatcher: it masks its index to a byte, drops indexes outside the
 * six-entry func_800C1804 pointer table, and tail-calls the selected entry,
 * whose own body calls func_801DB524 with 3 and then sets the 0x2 bit of
 * D_80146328. */
void func_800C1BF0(void);
void func_800C1C3C(void);
void func_800C1B84(u8 index);

/* Shared hook slots. 0x801463A4/0x801463A8 are the battle hooks the battle
 * step loads and calls (jalr); 0x801463AC is the third slot of the same trio,
 * written by every boss overlay of this archive family. */
extern Boss03516Handler D_801463A4; /* @source 0x801463A4 @kind unknown */
extern Boss03516Handler D_801463A8; /* @source 0x801463A8 @kind unknown */
extern Boss03516Handler D_801463AC; /* @source 0x801463AC @kind unknown */

/* @source 0x800C1B4C
 * @behavior Publishes this overlay's handler trio into the shared hook slots:
 * func_800C1BF0 (state switch: tests the 0x2 bit of D_80146328 and, while it is
 * set, stores 0x15 into the byte at D_80146875 and calls func_801DDED8, and
 * otherwise calls func_801DDF00) into 0x801463A4, func_800C1C3C (sets the 0x2
 * bit of the halfword at D_801462E8 and calls func_8014ECAC with 4) into
 * 0x801463A8, and the indexed handler dispatcher func_800C1B84 into 0x801463AC;
 * that dispatcher's byte index parameter is dropped by the void slot type. Same
 * trio shape as the sibling installers of this archive family, which write the
 * same three slots in the same order and differ only in the published
 * addresses. No in-payload caller: neither this function's address nor the
 * three slot addresses appear as a word of this payload and no jal reaches it,
 * so the main program reaches it through its overlay entry point.
 * @status exact
 * @match 100.00
 * @residual none
 */
void publishBattleHooksBossBoss03516_800C1B4C(void) {
  D_801463A4 = func_800C1BF0;
  D_801463A8 = func_800C1C3C;
  D_801463AC = (Boss03516Handler)func_800C1B84;
}
