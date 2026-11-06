#include "bof3/bof3.h"

/* Signed scenario-state byte (the first byte of the etc/game scenario-state
 * record), read here with `lb`, so the declaration keeps the s8 width that
 * include/bof3/ui/shop00_internal.h and include/bof3/world/area01613_internal.h
 * give it. */
extern s8 D_80146870; /* @source 0x80146870 @kind data */

/* Front-side byte published when the player position resolves into one of the
 * pending world id's bounding boxes; read here with `lbu`, so it stays u8. Bind
 * by the shared map row. */
extern u8 D_80143F03; /* @source 0x80143F03 @kind data */

/* Transition arm byte of this overlay family: the transition handler at
 * 0x801F33E4 sets it to 1 while the incoming trigger coordinates lie outside
 * its window and clears it while queueing cue 0x20C. Here it is cleared
 * unconditionally, so its role beyond that clear stays address-anchored. */
extern u8 D_801455C7; /* @source 0x801455C7 @kind data */

/* Shared record/effect setup routine at 0x801EECA4, resident in the
 * 0x801EEC00 overlay slot: it copies the words at 0x34/0x38/0x3C of the record
 * published at 0x8014638C into the scratchpad work object and allocates two
 * rows through func_801E590C. Unlifted in every overlay that carries it, so no
 * prototype is imported. */
void func_801EECA4(void);

/* Shared frontend cue dispatcher at 0x8015DF18; the cue id arrives
 * zero-extended, as the same declaration in
 * include/bof3/scenario/sce10eff_internal.h records. */
void func_8015DF18(u16 arg0);

/* @source 0x801F32D8
 * @behavior World-overlay area-module entry. When the signed scenario-state
 *   byte 0x80146870 reads at least 8 it first runs the shared record/effect
 *   setup routine 0x801EECA4; it then clears the byte 0x801455C7 and, when the
 *   front-side byte 0x80143F03 reads nonzero, requests frontend cue 0x20C
 *   through the shared frontend cue dispatcher 0x8015DF18. Takes no arguments
 *   and returns nothing.
 *
 * Name evidence: this payload holds no in-image caller of the address - the
 *   only word holding 0x801F32D8 is the area-descriptor field at 0x801F37DC,
 *   beside the descriptor's textbin word 0x801F3474 at 0x801F37BC and its
 *   in-overlay handler-table base 0x801F3720 at 0x801F37D0 - so the range is
 *   the module entry the world engine calls from outside the payload. The two
 *   gated steps are the whole body: the progress-gated setup call and the
 *   front-cue request. The name is the one the exact sibling
 *   emi/world04/area182/13 carries for this byte-identical 80-byte span,
 *   re-anchored to this target.
 *
 * Shape: payload span 0x6D8..0x728 is byte-identical to the exact source of
 *   emi/world04/area182/13 at the same load address (`bin/harness lift clone`
 *   reports byte_identical true over 80 bytes), and the same span matches areas
 *   176, 177, 178, 179, 180, 181, 184 and 185, whose rows are still unlifted
 *   `asm`. The span holds no address-specific immediate, so the sibling's
 *   statement order, declaration style and operand forms are reused and this
 *   target's own literals and bindings come from its original bytes: the
 *   `lb`/`slti 8` guard, the `lbu` test whose load the original keeps ahead of
 *   the unconditional byte clear, and the cue id materialised in the call's
 *   branch delay slot.
 * @status exact
 * @match 100.00
 * @residual none
 */
void initAreaModuleAndQueueFrontCue20CWorld04Area18313_801F32D8(void) {
  if (D_80146870 >= 8) {
    func_801EECA4();
  }
  D_801455C7 = 0;
  if (D_80143F03 != 0) {
    func_8015DF18(0x20C);
  }
}
