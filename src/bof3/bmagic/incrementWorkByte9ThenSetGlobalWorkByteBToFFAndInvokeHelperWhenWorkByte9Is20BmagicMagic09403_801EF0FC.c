#include "bof3/bof3.h"

void func_801E5988(void);

/* Scratchpad work-object pointer cell at 0x1F800044, bound by this target's map;
 * the remaining asm of this overlay spells the address as a literal, so Splat's
 * generated undefined_syms_auto.txt cannot bind it. The original reads the cell
 * once and every later work-object access goes through that loaded pointer, so
 * the plain named cell suffices. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared battle work-record pointer in main RAM, outside this overlay's image.
 * The remaining asm spans func_801EEE4C and func_801EF710 still reference the
 * address, so Splat's generated undefined_syms_auto.txt binds it and no
 * target-map row is needed. Byte 0xB of that record is the work step the exact
 * siblings of this overlay read and rewrite. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* @source 0x801EF0FC
 * @behavior Advances the scratchpad work object's byte at offset 0x9 by one
 *           modulo 256 through one load of the pointer cell at 0x1F800044
 *           (lbu / addiu 1 / sb), then tests the truncated byte it just stored
 *           (andi 0xFF of the stored register, no second load of the cell)
 *           against 0x20. Only when that byte equals 0x20 does it read the
 *           shared battle work-record pointer at 0x801EB4E0, store 0xFF into
 *           that record's byte at offset 0xB - that constant is materialised in
 *           the delay slot of the equality test - and invoke the shared helper
 *           at 0x801E5988, the store filling the call delay slot. The 0x18-byte
 *           frame only keeps $ra at 0x10 because of the call; the work pointer
 *           load is issued before the frame is created. It takes no arguments,
 *           returns nothing and reads or writes no other state.
 *
 *           Statement order, operand forms and declaration style follow the
 *           exact sibling
 *           incrementWorkByte9ThenSetGlobalWorkByteBToOneAndAdvanceWorkByte2At10BmagicMagic06000_801F0030
 *           (same pointer cell and u8 accumulator, same truncated-byte equality
 *           test on the value just stored, and the shared battle work-record
 *           byte 0xB written inside that arm) and the exact sibling
 *           subtractWorkColorTripletBy3ThenDecrementWorkByte9SetGlobalWorkByteB
 *           ToFFAndInvokeHelperWhenZeroBmagicMagic00800_801F08C4 (guard arm that
 *           stores 0xFF into the same record byte and calls the same helper,
 *           with that store in the call delay slot). This function's own
 *           operator, addend and test immediate (add 1, compare 0x20) and its
 *           helper call come from this target's original bytes. No extracted
 *           payload repeats this span: a byte scan of the 80-byte range over all
 *           394 extracted EMI payloads finds no second copy, so the
 *           discriminating immediate 0x20 appears only in this target's binary.
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementWorkByte9ThenSetGlobalWorkByteBToFFAndInvokeHelperWhenWorkByte9Is20BmagicMagic09403_801EF0FC(void) {
  u8 *work;
  u8 value;

  work = g_battle_work;
  value = work[9] + 1;
  work[9] = value;
  if (value == 0x20) {
    D_801EB4E0[0xB] = 0xFF;
    func_801E5988();
  }
}
