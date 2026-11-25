#include "bof3/bof3.h"

typedef void (*Boss04216Handler)(void);

/* Overlay entrypoints this installer publishes. The first is the state
 * handler at 0x800C193C, which tests bit 1 of D_80146328 and either stores 5
 * into D_80146875 and calls func_801DDED8, or calls func_801DDF00. The second
 * takes no argument and returns immediately. The third returns a zero result,
 * which the void slot type drops. */
void func_800C193C(void);
void noopHandler107(void);
s32 zeroResult21(void);

/* Shared hook slots. 0x801463A4 and 0x801463A8 are battle hooks the battle
 * step and the EMI stream starter load and call (jalr); 0x801463AC is the
 * third slot of the same trio, written by every boss overlay of this archive
 * family. */
extern Boss04216Handler D_801463A4; /* @source 0x801463A4 @kind unknown */
extern Boss04216Handler D_801463A8; /* @source 0x801463A8 @kind unknown */
extern Boss04216Handler D_801463AC; /* @source 0x801463AC @kind unknown */

/* @source 0x800C18FC
 * @behavior Publishes this overlay handler trio into the shared hook slots in
 * slot order: the state handler at 0x800C193C into 0x801463A4, the empty
 * handler noopHandler107 into 0x801463A8, and the
 * zero-result accessor zeroResult21 into 0x801463AC, the
 * last one through the void slot type. The 0x38-byte body reserves no frame
 * and keeps nothing across the three stores. No word of this payload names
 * this function and no jal reaches it, so the main program enters it through
 * the overlay entry point, as it does the sibling installer 0x800C18F0 of
 * BIN/BOSS/BOSS029.EMI#16, which writes the same three slots in the same
 * order.
 * @status exact
 * @match 100.00
 * @residual none
 */
void publishBattleHooks15(void) {
  D_801463A4 = func_800C193C;
  D_801463A8 = noopHandler107;
  D_801463AC = (Boss04216Handler)zeroResult21;
}
