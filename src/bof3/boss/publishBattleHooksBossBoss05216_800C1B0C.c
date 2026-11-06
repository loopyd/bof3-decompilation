#include "bof3/bof3.h"

typedef void (*Boss05216Handler)(void);

/* Overlay entrypoints this installer publishes. The first is the overlay's
 * state switch: it tests the 0x2 bit of D_80146328 and, while it is set, walks
 * three channel groups (D_80145E90/D_80145FD0/D_80146110), each publishing its
 * record pointer to the scratchpad work cell and calling func_8014D5F0, then
 * stores 0x1A into D_80146875 and calls func_801DDED8, and otherwise calls
 * func_801DDF00. The second is the setup routine: it calls func_800AC844(0) and
 * func_800AC754(0), publishes the result to D_80146884 and the scratchpad work
 * cell, then calls func_8014D6B8(5). The third returns a zero result, which the
 * void slot type drops. */
void func_800C1B4C(void);
void func_800C1D14(void);
s32  zeroResultBossBoss05216_800C1B44(void);

/* Shared hook slots. 0x801463A4/0x801463A8 are the battle hooks the battle
 * step loads and calls (jalr); 0x801463AC is the third slot of the same trio,
 * written by every boss overlay of this archive family. */
extern Boss05216Handler D_801463A4; /* @source 0x801463A4 @kind unknown */
extern Boss05216Handler D_801463A8; /* @source 0x801463A8 @kind unknown */
extern Boss05216Handler D_801463AC; /* @source 0x801463AC @kind unknown */

/* @source 0x800C1B0C
 * @behavior Publishes this overlay's handler trio into the shared hook slots:
 * func_800C1B4C (state switch: tests the 0x2 bit of D_80146328 and, while it is
 * set, walks three channel groups D_80145E90/D_80145FD0/D_80146110, each
 * publishing its record pointer to the scratchpad work cell and calling
 * func_8014D5F0, then stores 0x1A into D_80146875 and calls func_801DDED8, and
 * otherwise calls func_801DDF00) into 0x801463A4, the setup routine
 * func_800C1D14 (calls func_800AC844(0) and func_800AC754(0), publishes the
 * result to D_80146884 and the scratchpad work cell, then calls
 * func_8014D6B8(5)) into 0x801463A8, and the zero-result accessor
 * zeroResultBossBoss05216_800C1B44 into 0x801463AC; the accessor's s32 result
 * is dropped by the void slot type. Same trio shape as the sibling installers
 * of this archive family, which write the same three slots in the same order.
 * No in-payload caller: neither this function's address nor the three slot
 * addresses appear in any other word of this payload, and no jal reaches it, so
 * the main program reaches it through its overlay entry point.
 * @status exact
 * @match 100.00
 * @residual none
 */
void publishBattleHooksBossBoss05216_800C1B0C(void) {
  D_801463A4 = func_800C1B4C;
  D_801463A8 = func_800C1D14;
  D_801463AC = (Boss05216Handler)zeroResultBossBoss05216_800C1B44;
}
