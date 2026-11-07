#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell shared by the battle and bmagic
 * overlays. The original loads the cell once (lui $v0,0x1F80 / lw $v0,0x44($v0))
 * before the frame is created, so the cell is reached through this target-map
 * symbol rather than through a constant address, which the compiler would
 * materialize with a full 32-bit immediate. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared battle work record pointer in main RAM, outside this overlay's image.
 * splat binds 0x801EB4E0 through this target's generated
 * undefined_syms_auto.txt, so no target-map row is needed. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

void func_801E5988(void);

/* @source 0x801EF130
 * @behavior Per-frame handler of the work object published at the scratchpad
 *           pointer cell 0x1F800044. It increments the object's unsigned byte
 *           0xA with a byte-width read-modify-write (lbu v1,0xA(v0) / addiu 1 /
 *           sb v1,0xA(v0)) and compares the stored value with 0x80 (andi $v1,
 *           $v1,0xFF / addiu $v0,$zero,0x80 / bne), so the truncation supplies
 *           the compared value and the byte is not re-read. Only when it
 *           reaches 0x80 the handler reads the shared battle work record
 *           pointer at 0x801EB4E0, writes 0xFF into that record's byte 0x0B and
 *           invokes the shared helper at 0x801E5988, the 0xFF materialization
 *           filling the delay slot of the bne while the store fills the delay
 *           slot of the call. The single cell load precedes frame creation
 *           because the increment needs no stack; the 0x18-byte frame only
 *           keeps $ra across the call. Takes no arguments, returns nothing and
 *           reads or writes no other state. This entry is the sixth and last
 *           word of this overlay's six-entry handler run at 0x801EFC9C
 *           (0x801EEE0C, 0x801EEF48, 0x801EF020, 0x801EF090, 0x801EF0E0,
 *           0x801EF130), which the step dispatcher at 0x801EED7C selects from
 *           the work object's byte 1 (lbu 1 / sll 2 / lw / jalr), so no jal in
 *           the payload refers to this entry. Statement order, operand
 *           forms and declaration style follow the exact siblings
 *           incrementWorkByteAAndAdvanceWorkByte1At60BmagicMagic10803_801EF0E0
 *           (same cell, same byte 0xA read-modify-write, different compared
 *           constant and different arm) and
 *           incrementWorkByte9AndDecrementWorkByteAThenDecrementGlobalWorkByteB
 *           AndInvokeHelperWhenZeroBmagicMagic14503_801EF0A0 (same 0x18-byte
 *           frame, same 0x801EB4E0 record byte 0x0B arm and the same helper
 *           call). No byte-identical copy of this 0x50-byte span exists in any
 *           overlay payload of this repository's binaries.
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementWorkByteAThenSetGlobalWorkByteBToFFAt80AndInvokeHelperBmagicMagic10803_801EF130(void)
{
  u8 countdown;

  countdown = (u8)(g_battle_work[0xA] + 1);
  g_battle_work[0xA] = countdown;
  if (countdown == 0x80) {
    D_801EB4E0[0x0B] = 0xFF;
    func_801E5988();
  }
}
