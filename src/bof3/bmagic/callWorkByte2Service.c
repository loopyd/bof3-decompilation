#include "bof3/bof3.h"

typedef void (*BmagicMagic16103WorkByte2Handler)(void);

typedef struct BmagicMagic16103WorkByte2DispatchTable {
  BmagicMagic16103WorkByte2Handler handlers[5];
} BmagicMagic16103WorkByte2DispatchTable;

/*
 * Code pointer run at 0x801EEC10, the five-word table this dispatcher copies:
 * entry 0 = 0x801EF024, entry 1 = 0x801EF06C, entry 2 = 0x801EF0E8, entry 3 =
 * 0x801EF158 and entry 4 = 0x801EF1BC, all function starts of this overlay's
 * work byte 2 sequence. It is the second half of the eight-word run in the
 * overlay's leading data block, which no map row owns, so the reference is
 * bound by the target's generated undefined-symbol table.
 */
extern BmagicMagic16103WorkByte2DispatchTable D_801EEC10; /* @source 0x801EEC10 @kind data */

/* Scratchpad work-object pointer cell, shared by this overlay's handlers. The
 * original materialises the cell address (lui/lw) before the frame is created
 * and re-reads it after the indirect call, because the called handler may
 * publish a new work object. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared main-executable service at 0x8014D290; this caller passes no
 * arguments. Declared per caller, the way the other bmagic lifts do. */
void func_8014D290(void);

/* @source 0x801EEF80
 * @behavior Step dispatcher of this overlay reached as entry 0 of the one-word
 *           work byte 1 table at 0x801EF3E8, which the dispatcher at 0x801EEF3C
 *           indexes with the work object's unsigned byte 1. It reads the
 *           scratchpad work-object pointer cell at 0x1F800044 before its
 *           0x30-byte frame is created, copies the five-entry work byte 2
 *           handler run at 0x801EEC10 (0x801EF024, 0x801EF06C, 0x801EF0E8,
 *           0x801EF158, 0x801EF1BC) into that frame at 0x10 as inline word
 *           copies, then invokes the copied entry that the work object's
 *           unsigned byte 2 selects, with no arguments and no bounds test.
 *           After the handler returns it re-reads the cell and, when the work
 *           object's byte 0 and byte 2 are both non-zero, invokes the shared
 *           main-executable service at 0x8014D290. It takes no arguments and
 *           returns nothing; the frame keeps the table copy at 0x10 and $ra at
 *           0x28, and it reads or writes no other state. All 41 words are
 *           identical to the span at 0x801EFF1C of emi/bmagic/magic015/00
 *           except the copied run's base, 0x801EEC54 there against 0x801EEC10
 *           here (payload byte comparison: that body still sat on its asm row,
 *           so it supplied statement order and operand forms while this body
 *           writes its own table address).
 * @status exact
 * @match 100.00
 * @residual none
 */
void callWorkByte2Service(
    void) {
  BmagicMagic16103WorkByte2DispatchTable handlers;
  u8 *work;

  handlers = D_801EEC10;
  handlers.handlers[g_battle_work[2]]();
  work = g_battle_work;
  if (work[0] != 0 && work[2] != 0) {
    func_8014D290();
  }
}
