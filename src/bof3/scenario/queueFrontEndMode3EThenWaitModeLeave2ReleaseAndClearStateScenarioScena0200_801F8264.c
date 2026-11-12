#include "bof3/bof3.h"

/* Shared primary mode byte of the main executable; the body arms it with 2.
 * @source 0x80143BB0 @kind unknown */
extern u8 D_80143BB0;
/* Signed shared primary state byte of the main executable; the body clears it.
 * @source 0x80146874 @kind unknown */
extern s8 D_80146874;
/* Shared sub-state byte of the main executable.
 * @source 0x80146875 @kind unknown */
extern u8 D_80146875;

/* Front-end mode request of the main executable, outside this payload: it
 * consumes the caller mode id and this body sets nothing but $a0 for it. */
void func_80150224(u32 arg0);
/* Shared overlay release helper at 0x8015C058. */
void func_8015C058(void);

/* @source 0x801F8264
 * @behavior Runs two states over the shared sub-state byte D_80146875
 * (0x80146875), which it reads once through one address taken into a local
 * pointer and therefore keeps in $s0 for the whole body. State 0 queues the
 * front-end mode 0x3E through the shared request func_80150224, arms the shared
 * primary mode byte D_80143BB0 (0x80143BB0, read with lbu in state 1) with 2
 * and advances the sub-state byte to 1. State 1, once that mode byte leaves 2,
 * runs the shared overlay release helper func_8015C058 and clears the sub-state
 * byte and the signed shared primary state byte D_80146874 (0x80146874) back to
 * zero, ending the state. Every other sub-state value returns without touching
 * the tail, and both moving states end in the epilogue. It takes no arguments
 * and returns nothing; its 0x18-byte frame keeps only $ra and $s0, the two case
 * constants 1 and 2 stay in $v0, and the state-0 store writes the literal 1
 * that the original materialises with an addiu of $zero rather than an
 * increment of the loaded byte. No in-image jal targets 0x801F8264: the only
 * word in the payload equal to that address is at 0x801FE2A8, eight words into
 * the code-pointer run that begins at 0x801FE288, the table
 * dispatchProgressHandlerScenarioScena0200_801F719C indexes with the signed
 * chain byte D_80146872, so the overlay reaches this span as a table slot. Its
 * 0x90 bytes have no byte-identical copy in the tree: a scan of every payload
 * under out/binaries that compares all 36 words with only the j and jal targets
 * masked matched this span alone, so it was seeded from the same-target
 * statement order and cell names of the exact siblings
 * armProgress0UnlessMode2ThenSetFlagBit13OrSeedFrontSelectorContext76AndClearStateScenarioScena0900_801FA08C
 * and queueFrontEndModeAndRequestPrimaryMode2ScenarioScena0400_801F9FD0.
 * @status exact
 * @match 100.00
 * @residual none
 */
void queueFrontEndMode3EThenWaitModeLeave2ReleaseAndClearStateScenarioScena0200_801F8264(void) {
  u8 *state = &D_80146875;

  switch (*state) {
  case 0:
    func_80150224(0x3E);
    D_80143BB0 = 2;
    *state = 1;
    return;
  case 1:
    if (D_80143BB0 == 2u) {
      return;
    }
    func_8015C058();
    *state = 0;
    D_80146874 = 0;
    return;
  }
}
