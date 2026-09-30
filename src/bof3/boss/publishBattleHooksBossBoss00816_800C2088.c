#include "bof3/bof3.h"

typedef void (*Boss00816Handler)(void);

/* Overlay entrypoints this installer publishes; the first two return void,
 * the third is the overlay's zero-result accessor. */
void func_800C20C8(void);
void func_800C2114(void);
s32  zeroResultBossBoss00816_800C20C0(void);

/* Shared hook slots. 0x801463A4/0x801463A8 are the battle hooks the battle
 * step loads and calls (jalr); 0x801463AC is the third slot of the same trio,
 * written by every boss overlay of this archive family. */
extern Boss00816Handler D_801463A4; /* @source 0x801463A4 @kind unknown */
extern Boss00816Handler D_801463A8; /* @source 0x801463A8 @kind unknown */
extern Boss00816Handler D_801463AC; /* @source 0x801463AC @kind unknown */

/* @source 0x800C2088
 * @behavior Publishes this overlay's second handler trio into the shared hook
 * slots: the progress-mode switch 0x800C20C8 (writes scenario progress 0x20)
 * into 0x801463A4, the setup routine 0x800C2114 (initializes mode work with
 * argument 1) into 0x801463A8, and the zero-result accessor 0x800C20C0 into
 * 0x801463AC; the accessor is stored through the slot type, so its s32 result
 * is never consumed. Reached as one entry of the battle mode table D_800B5108,
 * which the battle engine dispatches by the battle mode byte.
 * @status exact
 * @match 100.00
 * @residual none
 */
void publishBattleHooksBossBoss00816_800C2088(void) {
  D_801463A4 = func_800C20C8;
  D_801463A8 = func_800C2114;
  D_801463AC = (Boss00816Handler)zeroResultBossBoss00816_800C20C0;
}
