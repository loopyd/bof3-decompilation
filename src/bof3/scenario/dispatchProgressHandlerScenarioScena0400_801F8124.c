#include "bof3/bof3.h"

extern s8 D_80146872;
extern u32 D_801FA204[];

/* @source 0x801F8124
 * @behavior Dispatches one frame of this overlay's state chain from the signed
 * shared scenario state byte D_80146872 (0x80146872): the byte is scaled by
 * four and selects an entry of the overlay-local handler table at 0x801FA204
 * (its first three words are 0x801F8160, 0x801F817C and 0x801F863C, the state
 * handlers the byte advances 0 -> 1 -> 2), and the selected entry is then
 * invoked with `jalr` and no arguments. It reads no state other than that byte
 * and writes none; the 0x18-byte frame exists only to keep $ra across the
 * indirect call. Entry 0 of that table (0x801F8160) names this address as its
 * dispatcher, its three table words are the state handlers it advances, and the
 * shape is the same one the D_80146872 progress dispatchers of the other EMI
 * scenario overlays use (dispatchProgressHandler_scena18 0x801F6C04,
 * dispatchProgressHandlerScenarioScena0100_801F7FC4 0x801F7FC4), hence the
 * progress-handler dispatch name; the address anchor keeps it target-qualified.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchProgressHandlerScenarioScena0400_801F8124(void) {
  ((void (*)(void))D_801FA204[(s8)D_80146872])();
}
