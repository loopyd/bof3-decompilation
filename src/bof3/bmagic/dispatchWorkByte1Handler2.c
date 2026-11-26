#include "bof3/bof3.h"

typedef void (*BmagicMagic15403WorkByte1Handler)(void);

typedef struct BmagicMagic15403WorkByte1DispatchTable {
  BmagicMagic15403WorkByte1Handler handlers[4];
} BmagicMagic15403WorkByte1DispatchTable;

/*
 * Code pointer run at 0x801EEC04, the first four words of the eight-word block
 * this overlay's leading data holds: entry 0 = 0x801EEC90, entry 1 = 0x801EEE94,
 * entry 2 = 0x801EEFE0, entry 3 = 0x801EF060. The four handler blocks after them
 * start at the next overlay words, and the run's second half at 0x801EEC14 is the
 * four-entry table the sibling dispatcher at 0x801EF4B4 copies, so this
 * declaration covers only the words this function copies. The address sits in
 * the overlay's leading data block, which no map row owns, so the reference is
 * bound by this target's generated undefined-symbol table.
 */
extern BmagicMagic15403WorkByte1DispatchTable D_801EEC04; /* @source 0x801EEC04 @kind data */

/*
 * Scratchpad work-record pointer cell, outside this overlay's image and read
 * before the frame below is created. This target's map binds no name there, so
 * the cell is reached through its address-anchored name.
 */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind data */

/* @source 0x801EEC24
 * @behavior Work-record byte 1 handler dispatcher of this overlay, the plain
 *           copy-and-call shape the sibling dispatcher at 0x801EF4B4 shows with
 *           its trailing helper call removed: it reads the work-record pointer
 *           cell at 0x1F800044 once before its frame exists, copies the
 *           four-entry code pointer run at 0x801EEC04 (0x801EEC90, 0x801EEE94,
 *           0x801EEFE0, 0x801EF060) into its own 0x28-byte frame at 0x10 as four
 *           word copies, and invokes the copied entry selected by that record's
 *           unsigned byte 1 scaled by four, with no arguments and no bounds
 *           test, so the call target is read back from the copy. It takes no
 *           arguments, returns nothing, and reads or writes no other state; the
 *           frame keeps the four-word copy at 0x10 and $ra at 0x20.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte1Handler(void) {
  BmagicMagic15403WorkByte1DispatchTable handlers;

  handlers = D_801EEC04;
  handlers.handlers[D_1F800044[1]]();
}
