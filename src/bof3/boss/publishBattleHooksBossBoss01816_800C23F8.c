#include "bof3/bof3.h"

typedef void (*Boss01816Handler)(void);

/* Overlay entrypoints this installer publishes. The first two are void
 * routines; the third is the overlay's mode dispatcher, which reads a byte
 * index in $a0, so it is stored through the void slot type. */
void func_800C253C(void);
void func_800C265C(void);
void func_800C2430(u8 index);

/* Shared hook slots. 0x801463A4/0x801463A8 are the battle hooks the battle
 * step loads and calls (jalr); 0x801463AC is the third slot of the same trio,
 * written by every boss overlay of this archive family. */
extern Boss01816Handler D_801463A4; /* @source 0x801463A4 @kind unknown */
extern Boss01816Handler D_801463A8; /* @source 0x801463A8 @kind unknown */
extern Boss01816Handler D_801463AC; /* @source 0x801463AC @kind unknown */

/* @source 0x800C23F8
 * @behavior Publishes this overlay's second handler trio into the shared hook
 * slots: func_800C253C (progress switch: reads the D_80146328 bit 0x2, selects
 * progress 0x1A/0x23 from the D_801462ED bit 0x2 or 0x1C/0x3A from
 * func_8015B5D4 probes, stores it to D_80146875 and copies four halfwords into
 * D_801FD6FC..D_801FD702) into 0x801463A4, func_800C265C (setup: starts the
 * 0x801EB630 segments through func_800AC870 and flushes with func_800AC844)
 * into 0x801463A8, and the mode dispatcher func_800C2430 (six-entry jump table
 * at 0x800C181C) into 0x801463AC; the dispatcher's index argument is dropped by
 * the void slot type. Same trio shape as the sibling installers 0x800C2114 and
 * 0x800C26DC of this overlay, which write the same three slots in the same
 * order. No in-payload caller: the main program reaches it through its overlay
 * entry point.
 * @status exact
 * @match 100.00
 * @residual none
 */
void publishBattleHooksBossBoss01816_800C23F8(void) {
  D_801463A4 = func_800C253C;
  D_801463A8 = func_800C265C;
  D_801463AC = (Boss01816Handler)func_800C2430;
}
