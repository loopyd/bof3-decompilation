#include "bof3/bof3.h"

typedef void (*Boss03113Handler)(void);

/* Overlay entrypoints this installer publishes. The first is this overlay's
 * battle handler at 0x800C1A08, which this lane leaves on its assembly row, so
 * its role stays address-anchored. The second is the overlay's battle setup
 * hook, which strips the 0x40 flag of owner record 0, publishes that record into
 * 0x80146884 and the scratchpad work-record cursor 0x1F800044, requests
 * resource 1 and then sets the work record byte at offset 0x2A. The third is an
 * inert accessor returning a zero result, which the void slot type drops. */
void dispatchBattleSetupHook(void); /* @source 0x800C1A08 */
void publishOwnerRequestResource(
    void); /* @source 0x800C1A6C */
s32 zeroResult35(void); /* @source 0x800C1A00 */

/* Shared hook slots. 0x801463A4/0x801463A8 are the battle hooks the battle step
 * loads and calls (jalr); 0x801463AC is the third slot of the same trio,
 * written by every boss overlay of this archive family. */
extern Boss03113Handler D_801463A4; /* @source 0x801463A4 @kind unknown */
extern Boss03113Handler D_801463A8; /* @source 0x801463A8 @kind unknown */
extern Boss03113Handler D_801463AC; /* @source 0x801463AC @kind unknown */

/* @source 0x800C19C8
 * @behavior Publishes this overlay's handler trio into the shared battle hook
 * slots: dispatchBattleSetupHook into 0x801463A4, the battle setup hook
 * publishOwnerRequestResource
 * into 0x801463A8, and the zero-result accessor
 * zeroResult35 into 0x801463AC; that accessor's s32 result
 * is dropped by the void slot type. Each slot takes a `lui`/`addiu` pair
 * materializing the published address into $v0 and one `sw` through $at, in
 * ascending slot order, in a leaf body with no frame that returns immediately.
 * No in-payload caller: neither this function's address nor the three slot
 * addresses appear as a word of this payload and no jal reaches it, so the main
 * program reaches it through its overlay entry point.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Byte evidence: the 0x38-byte span is the same 14 words as the exact installer
 * at 0x800C19C8 of BIN/BOSS/BOSS032.EMI#16 except word 0x1DC, the `addiu` %lo of
 * the published setup routine, which names 0x1A6C here and 0x1BEC there from
 * the same `lui 0x800C`; every slot word, the third published address and the
 * `jr $ra`/`nop` tail are equal.
 */
void publishHooksOwnerRequest(void) {
  D_801463A4 = dispatchBattleSetupHook;
  D_801463A8 = publishOwnerRequestResource;
  D_801463AC = (Boss03113Handler)zeroResult35;
}
