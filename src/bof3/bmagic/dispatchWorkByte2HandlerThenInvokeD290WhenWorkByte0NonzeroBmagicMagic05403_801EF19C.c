#include "bof3/bof3.h"

typedef void (*BmagicMagic05403Handler)(void);

typedef struct BmagicMagic05403WorkByte2Table {
  BmagicMagic05403Handler handlers[6];
} BmagicMagic05403WorkByte2Table;

/*
 * Code pointer run at 0x801EEC14, the six-entry table this dispatcher copies:
 * entry 0 = 0x801EF238, entry 1 = 0x801EF3A4, entry 2 = 0x801EF43C,
 * entry 3 = 0x801EF4A8, entry 4 = 0x801EF534, entry 5 = 0x801EF624, the six
 * work byte 2 step handlers of this overlay. It begins at the word after the
 * four entries the sibling dispatcher at 0x801EEC2C copies, the target map
 * binds it, and the target's generated undefined-symbol table already covered
 * the address as D_801EEC14 before this lift.
 */
extern BmagicMagic05403WorkByte2Table dispatchWorkByte2HandlerTable801EEC14; /* @source 0x801EEC14 @kind data */

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
 * per caller.
 */
extern void func_8014D290(void); /* @source 0x8014D290 @kind function */

/* @source 0x801EF19C
 * @behavior Work byte 2 step dispatcher of the overlay work object. It reads
 * the work-object pointer cell at 0x1F800044 once before its frame exists,
 * copies the six-entry work byte 2 handler table at 0x801EEC14 (0x801EF238,
 * 0x801EF3A4, 0x801EF43C, 0x801EF4A8, 0x801EF534, 0x801EF624) into its own
 * 0x30-byte frame at 0x10 as six word copies, and invokes the copied entry
 * selected by that object's unsigned byte 2 scaled by four, with no arguments
 * and no bounds test. After the handler returns it reads the pointer cell a
 * second time and, when the work object's byte 0 is non-zero, invokes the
 * shared main-executable helper at 0x8014D290. It takes no arguments and
 * returns nothing; the frame keeps the six-word copy at 0x10 and $ra at 0x28,
 * and it reads and writes no other state. Its only in-image reference is the
 * pointer word 0x801EF19C that ends the payload at 0x801EF658, so the overlay
 * loader installs it.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte2HandlerThenInvokeD290WhenWorkByte0NonzeroBmagicMagic05403_801EF19C(
    void) {
  BmagicMagic05403WorkByte2Table handlers;

  handlers = dispatchWorkByte2HandlerTable801EEC14;
  handlers.handlers[D_1F800044[2]]();
  if (D_1F800044[0] != 0) {
    func_8014D290();
  }
}
