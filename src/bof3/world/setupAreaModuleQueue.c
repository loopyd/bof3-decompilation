#include "bof3/bof3.h"

/* Signed scenario-state byte (the first byte of the etc/game scenario-state
 * record), read here with `lb`, so the declaration keeps the s8 width that
 * include/bof3/ui/shop00_internal.h and include/bof3/world/area01613_internal.h
 * give it. */
extern s8 D_80146870; /* @source 0x80146870 @kind data */

/* Front-side byte published by the front-selector updates (reviewed as the
 * "front-side byte D_80143F03" by the scenario annotations); read with `lbu`
 * here, so it stays u8. Bind by the shared map row. */
extern u8 D_80143F03;

/* Byte cleared unconditionally on every entry, and the only address this
 * target never names elsewhere; the neighbouring handler func_801F33E4 stores
 * it. The role stays unknown and the name stays address-anchored. */
extern u8 D_801455C7; /* @source 0x801455C7 @kind data */

/* Shared record/effect setup routine at 0x801EECA4, resident in the shared
 * main executable: it copies the words at 0x34/0x38/0x3C of the record
 * published at 0x8014638C into the scratchpad work object and allocates two
 * rows through func_801E590C. It is unlifted in its own bmagic overlays, so no
 * prototype is imported from there. */
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
 * Name evidence: this payload holds no in-image caller of the address - no
 *   branch, jump or other code word targets 0x801F32D8, and the only word
 *   holding it is the payload's last word 0x801F37AC, the area-descriptor field
 *   beside the descriptor's textbin word 0x801F3474 at 0x801F378C and its
 *   in-payload handler-table base 0x801F36F0 at 0x801F37A0 (a run of handler
 *   addresses from 0x801F2C18 on) - so the range is the module entry the world
 *   engine calls from outside the payload. The two gated steps are the whole
 *   body: the progress-gated setup call and the front-cue request. The name is
 *   the one the exact siblings emi/world04/area177/13, area178/13, area180/13,
 *   area182/13, area183/13 and area184/13 carry for this byte-identical
 *   80-byte span, re-anchored to this target.
 *
 * Shape: payload span 0x6D8..0x728 is byte-identical to the exact source of
 *   emi/world04/area182/13 at the same load address (`bin/harness lift clone`
 *   reports byte-identical over 80 bytes), and the same span compares equal in
 *   areas 177, 178, 179, 180, 181, 183, 184 and 185. The span holds no
 *   address-specific immediate, so the sibling's statement order, declaration
 *   style and operand forms are reused and this target's own literals and
 *   bindings come from its original bytes: the `lb`/`slti 8` guard, the `lbu`
 *   test whose load the original keeps ahead of the unconditional byte clear,
 *   and the cue id materialised in the call's branch delay slot.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setupAreaModuleQueue(void) {
  if (D_80146870 >= 8) {
    func_801EECA4();
  }
  D_801455C7 = 0;
  if (D_80143F03 != 0) {
    func_8015DF18(0x20C);
  }
}
