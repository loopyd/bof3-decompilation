#include "bof3/bof3.h"

typedef void (*BmagicMagic15403Byte2Handler)(void);

typedef struct BmagicMagic15403Byte2DispatchTable {
  BmagicMagic15403Byte2Handler handlers[4];
} BmagicMagic15403Byte2DispatchTable;

/*
 * Code pointer run word at 0x801EEC14, the four-entry work byte 2 handler table
 * this dispatcher copies: entry 0 = 0x801EF544, entry 1 = 0x801EF58C, entry 2 =
 * 0x801EF648, entry 3 = 0x801EF6AC, the four work byte 2 step handlers of this
 * overlay. It is the second half of the eight-word run the overlay's leading
 * data block holds at 0x801EEC04 (0x801EEC90, 0x801EEE94, 0x801EEFE0,
 * 0x801EF060, then these four words), so this extern covers only the copied
 * words.
 */
extern BmagicMagic15403Byte2DispatchTable dispatchWorkByte2HandlerTable801EEC14; /* @source 0x801EEC14 @kind data */

/*
 * Scratchpad work-object pointer cell, outside this overlay's image and read
 * before each of the two accesses below. This target's map binds no name there,
 * so the cell is reached through its address-anchored name and the compiler
 * re-materialises the constant cell address for every access, which is the
 * original's lui/lw pair rather than a kept-base accessor.
 */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind data */

/*
 * Shared main-executable helper this overlay calls with no arguments; declared
 * per caller, like the other callers of the same address in this directory.
 */
extern void func_8014D290(void); /* @source 0x8014D290 @kind function */

/* @source 0x801EF4B4
 * @behavior Work byte 2 step dispatcher of this overlay's work object, reached
 *           as entry 1 of the code pointer run at 0x801EF6CC (0x801EF104,
 *           0x801EF4B4, 0x801EF1EC, 0x801EF380, 0x801EF3E4, 0x801EF454) that
 *           the exact lift dispatchStateByte1Tablef6ccBmagicMagic15403_801EF0C0
 *           indexes with work-object byte 1. It reads the work-object pointer
 *           cell at 0x1F800044 once before its frame exists, copies the
 *           four-entry work byte 2 handler table at 0x801EEC14 (0x801EF544,
 *           0x801EF58C, 0x801EF648, 0x801EF6AC) into its own 0x28-byte frame at
 *           0x10 as four word copies, and invokes the copied entry selected by
 *           that object's unsigned byte 2 scaled by four, with no arguments and
 *           no bounds test, so the call target is read back from the copy. After
 *           the handler returns it reads the pointer cell a second time and,
 *           when the work object's byte 0 is non-zero, invokes the shared
 *           main-executable helper at 0x8014D290. It takes no arguments and
 *           returns nothing; the frame keeps the four-word copy at 0x10 and $ra
 *           at 0x20, and it reads and writes no other state.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte2HandlerThenInvokeD290WhenWorkByte0NonzeroBmagicMagic15403_801EF4B4(
    void) {
  BmagicMagic15403Byte2DispatchTable handlers;

  handlers = dispatchWorkByte2HandlerTable801EEC14;
  handlers.handlers[D_1F800044[2]]();
  if (D_1F800044[0] != 0) {
    func_8014D290();
  }
}
