#include "bof3/bof3.h"

/* PsyQ libc random source at 0x8017E3D4, the address this body's jal word
 * encodes. The answer is discarded: the call's $v0 is dead, so the declaration
 * carries the library's return type with no use in this function. */
extern int rand(void);

/* Shared battle work record pointer in main RAM, outside this overlay's image.
 * The generated undefined_syms_auto.txt already binds the address, so no
 * target-map row is needed. The exact sibling
 * incrementWorkByte9AndDecrementWorkByteAThenDecrementGlobalWorkByteBAnd
 * InvokeHelperWhenZeroBmagicMagic14503_801EF0A0 decrements the same record byte
 * 0x0B and reaches the same helper. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Shared main-executable helper at 0x801E5988, reached by the 0x0C jal word in
 * this overlay. Its body is outside every payload lifted here, so it keeps its
 * raw name. */
void func_801E5988(void);

/* @source 0x801F0638
 * @behavior Entry 2 of the three-entry handler table at 0x801F0A2C that this
 *           overlay's work-byte-2 dispatcher
 *           dispatchWorkByte2HandlerThenDrawWhenActiveBmagicMagic04403_801F0478
 *           indexes with the scratchpad work object's byte 0x2, so no jal in
 *           this payload reaches this address; the table words
 *           0x801F04F0/0x801F05AC/0x801F0638 are the three state blocks that
 *           follow that dispatcher. It reads the scratchpad work pointer cell
 *           0x1F800044 once before it creates its frame and, through that one
 *           base, subtracts two from the work object's unsigned halfword 0x30
 *           and adds one to its byte 9 (lhu / lbu, addiu -2 / addiu 1, sh / sb),
 *           the byte store filling the rand() call delay slot. When rand()
 *           returns it re-reads the cell and subtracts one from the object's
 *           unsigned halfword 0x2E (lhu / addiu -1 / sh). It re-reads the cell
 *           a third time, subtracts one from unsigned byte 0x0A and tests the
 *           stored byte truncated to eight bits (andi $v0,$v0,0xFF / bnez), so
 *           the truncated register - not a reload - supplies the compared value;
 *           only when byte 0x0A reaches zero does it read the shared battle work
 *           record pointer at 0x801EB4E0, decrement that record's byte 0x0B by
 *           one and invoke the shared helper at 0x801E5988, the store filling
 *           the call delay slot. Every other byte-0x0A value branches straight
 *           to the epilogue. Takes no arguments, returns nothing, and the
 *           0x18-byte frame keeps only $ra at 0x10. The roles of the halfword
 *           pair 0x30/0x2E and of work bytes 9 and 0x0A remain unestablished, so
 *           the name stays offset-anchored.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementWorkHalfword30ByTwoAndHalfword2EByOneAdvanceByte9By1WhenByteAZeroDecrementGlobalWorkByteBAndInvokeHelperBmagicMagic04403_801F0638(void) {
  u8 **slots;
  u8 *work;
  u16 *halves;

  /* The cell is read once here and re-read for each later access group: the
   * original materialises 0x1F800044 with its own lui/lw pair at 0x1A38, 0x1A64
   * and 0x1A80, because each store issued through the loaded pointer may alias
   * the cell and the intervening rand() call invalidates it outright. The plain
   * scratchpad accessor gives one base per statement group without a volatile
   * qualifier; the same spelling carries the exact sibling
   * incrementWorkByte9AndDecrementWorkByteAThenDecrementGlobalWorkByteBAnd
   * InvokeHelperWhenZeroBmagicMagic14503_801EF0A0. */
  slots = SPAD_PTR_TABLE(u8);
  work = slots[0x11];
  halves = (u16 *)work;
  halves[0x18] -= 2;
  work[9] += 1;
  rand();
  halves = (u16 *)slots[0x11];
  halves[0x17] -= 1;
  /* --slots[0x11][0x0A] keeps the stored byte in the register and tests it
   * truncated (andi 0xFF) instead of reloading the cell a fourth time. */
  if (--slots[0x11][0x0A] == 0) {
    D_801EB4E0[0xB] -= 1;
    func_801E5988();
  }
}
