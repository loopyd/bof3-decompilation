#include "bof3/bof3.h"

typedef void (*BmagicMagic00903WorkByte2Handler)(void);

typedef struct BmagicMagic00903WorkByte2Table {
  BmagicMagic00903WorkByte2Handler handlers[4];
} BmagicMagic00903WorkByte2Table;

/*
 * Code pointer run at 0x801EEC18, the four-entry table this dispatcher copies:
 * entry 0 = 0x801EF740, entry 1 = 0x801EF788, entry 2 = 0x801EF814 and entry 3
 * = 0x801EF87C, the four work byte 2 step handlers of this overlay. It is the
 * tail of the nine-word run that begins at 0x801EEC04, sits in the overlay's
 * leading data block, and is bound by the target's generated undefined-symbol
 * table, which already spells this address.
 */
extern BmagicMagic00903WorkByte2Table D_801EEC18; /* @source 0x801EEC18 @kind data */

/*
 * Scratchpad work-object pointer cell shared by the battle and bmagic overlays;
 * this target's map binds it at 0x1F800044. The original re-materialises the
 * cell with its own lui/lw pair for both access groups, the first before its
 * frame exists and the second after the indirect call, which may republish the
 * cell, so each access stays on the named cell rather than on a cached pointer
 * local.
 */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/*
 * Shared main-executable helper this overlay calls with no arguments; declared
 * per caller and already covered by the target's generated undefined-funcs map.
 */
extern void func_8014D290(void); /* @source 0x8014D290 @kind function */

/* @source 0x801EF6B0
 * @behavior Work byte 2 step dispatcher with a reset pass of the overlay work
 *   object. It reads the work-object pointer cell at 0x1F800044 once before its
 *   frame exists, copies the four-entry work byte 2 handler run at 0x801EEC18
 *   (0x801EF740, 0x801EF788, 0x801EF814, 0x801EF87C) into its own 0x28-byte
 *   frame at 0x10 as three word moves followed by one, and invokes the copied
 *   entry selected by that object's unsigned byte 2 scaled by four, with no
 *   arguments and no bounds test. After the handler returns it reads the
 *   pointer cell a second time and, when the work object's byte 0 is non-zero,
 *   invokes the shared main-executable helper at 0x8014D290. It takes no
 *   arguments and returns nothing; the frame keeps the four-word copy at 0x10
 *   and $ra at 0x20, and it reads or writes no other state. No word of this
 *   payload holds its own address and no jal reaches it, so the caller lies
 *   outside the overlay and the argument roles stay structural.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte2Reset(
    void) {
  BmagicMagic00903WorkByte2Table handlers;

  handlers = D_801EEC18;
  handlers.handlers[g_battle_work[2]]();
  if (g_battle_work[0] != 0) {
    func_8014D290();
  }
}
