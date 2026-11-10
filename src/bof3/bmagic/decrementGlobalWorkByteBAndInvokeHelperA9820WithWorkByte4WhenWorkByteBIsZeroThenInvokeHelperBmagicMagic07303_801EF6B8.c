#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell. The compiled original materializes it as
 * `lui $v0,0x1F80` / `lw $v0,0x44($v0)` twice: once for the guard, before the
 * 0x18-byte frame is created, and again after the record byte store, because a
 * store through the pointer loaded from 0x801EB4E0 may alias this cell object.
 * The cell is reached through this address-derived symbol, which keeps the
 * constant-address `lui` + offset load form the original spells. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind data */

/* Shared battle work record pointer in main RAM, outside this overlay's image,
 * bound through this target's generated undefined_syms_auto.txt. The exact
 * lifts of this target read the same record through this pointer and decrement
 * the same byte 0xB. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Main-executable helper reached with the zero-extended work object byte 0x04.
 * It lives outside this overlay, so it keeps its raw name. */
void func_800A9820(u8 slot);

/* Shared helper of this overlay family, outside this overlay's image. Its role
 * is not established, so it stays address-anchored. */
void func_801E5988(void);

/* @source 0x801EF6B8
 * @behavior Handler of the scratchpad work object, the sixth and last of the
 *           six code pointers in the run at 0x801EEC04
 *           (0x801EF06C, 0x801EF2DC, 0x801EF3D4, 0x801EF4E4, 0x801EF52C,
 *           0x801EF6B8) that the byte-2 dispatcher func_801EEFF4 copies into
 *           its own 0x30-byte frame and indexes with the work object unsigned
 *           byte 2, so it runs with no arguments; the word at 0x801EEC18 is the
 *           only word in the payload that names this address and no `jal` does,
 *           and that dispatcher is the only in-payload reader of the run. The
 *           body loads the work-object pointer cell at 0x1F800044 once, before
 *           the 0x18-byte frame is created, and tests the work object unsigned
 *           byte 0x0B for zero with an equality test on the loaded byte. Only
 *           when that byte is zero does it load the shared battle work record
 *           pointer at 0x801EB4E0 and decrement that record byte 0x0B by one at
 *           byte width (lbu 0xB / addiu -1 / sb 0xB), the stored value never
 *           tested. It then re-reads the 0x1F800044 cell, because the store
 *           through the record pointer may alias the cell object, and passes the
 *           work object unsigned byte 0x04 to the main-executable helper at
 *           0x800A9820, and finally invokes the shared helper at 0x801E5988 with
 *           no arguments. Both call delay slots are `nop`, so neither statement
 *           is scheduled into them. The frame keeps only $ra at 0x10, the body
 *           returns through the `jr $ra` at 0x801EF71C and writes no other
 *           state; the roles of work byte 0x0B, work byte 0x04 and record byte
 *           0x0B are not established, so they stay offset-anchored. The 0x6C-byte
 *           span was byte-identical to the still-unlifted func_801EF6A4 of
 *           emi/bmagic/magic074/03, proved against every extracted payload under
 *           out/binaries, so that twin had no source to clone and supplied none;
 *           the statement order, operand forms, offset spelling and declaration
 *           style follow the exact lift
 *           invokeHelperA9820WithWorkByte4WhenWorkByteBIsZeroThenDecrement
 *           GlobalWorkByteBAndInvokeHelperBmagicMagic06303_801EF2E4, whose
 *           guard, cell load before frame creation and two helper calls match
 *           this span while its arm calls the 0x800A9820 helper before the
 *           record decrement, so the order written here is the one this span
 *           own bytes show.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementGlobalWorkByteBAndInvokeHelperA9820WithWorkByte4WhenWorkByteBIsZeroThenInvokeHelperBmagicMagic07303_801EF6B8(
    void) {
  if (D_1F800044[0x0B] == 0) {
    D_801EB4E0[0x0B]--;
    func_800A9820(D_1F800044[0x04]);
    func_801E5988();
  }
}
