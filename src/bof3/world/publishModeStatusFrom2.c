#include "bof3/bof3.h"

extern s8 D_801490C7;
extern u16 D_801490A8;
extern u16 D_801F43FC[]; /* @source 0x801F43FC @kind table */
extern u8 g_ScenarioProgress;

/* @source 0x801F2D2C
 * @behavior Overlay mode handler: reads the shared mode byte 0x801490C7 once
 *           with `lui`/`lb` (signed byte) and keeps it in a register across the
 *           table load, indexes this payload's halfword table 0x801F43FC by
 *           `sll` one and `addu` into $at, and publishes the fetched halfword
 *           into the shared status halfword 0x801490A8 with `sh`. It then tests
 *           the same byte with `bnez` and, only while that byte is zero, stores
 *           0x06 into the shared scenario progress byte 0x80146864; the branch
 *           delay slot carries the constant materialisation. No arguments, no
 *           calls, no other stores, leaf with no frame and one exit.
 *
 *           Evidence for the table: the halfwords at file offset 0x17FC read
 *           0xFFFF, 0x000B and 0x4722, so the base is a target-local lookup
 *           table rather than code, and no source in the tree binds it.
 *
 *           Evidence for the entry: this body is word 3 of the pointer run that
 *           starts at 0x801F4348 in this payload (file offset 0x1754 holds
 *           0x801F2D2C), so the dispatch index that selects it is unevidenced
 *           and its argument roles stay structural. Shape only: this 0x3C-byte
 *           span differs from the exact
 *           publishModeStatusMode
 *           span in one byte, the `%lo` of that sibling's table base (0xF8
 *           against 0xFC here), so its statement order and operand forms were
 *           transcribed with this span's own table binding. A scan of the 795
 *           payloads under out/binaries found no byte-identical copy of this
 *           span, which declined the clone lever.
 * @status exact
 * @match 100.00
 * @residual none
 */
void publishModeStatusFrom(
    void) {
  s32 mode;

  mode = D_801490C7;
  D_801490A8 = D_801F43FC[mode];
  if (mode == 0) {
    g_ScenarioProgress = 6;
  }
}
