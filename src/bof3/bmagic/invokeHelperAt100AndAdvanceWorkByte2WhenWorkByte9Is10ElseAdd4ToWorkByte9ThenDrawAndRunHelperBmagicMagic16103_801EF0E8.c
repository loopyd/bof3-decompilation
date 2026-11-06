#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell, shared by this overlay's handlers. The
 * original materialises the cell address (lui $a1,0x1F80 / lw $a1,68($a1))
 * before the frame is created and re-reads it after the helper call, because
 * the call may publish a new work object. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Main-executable helper at 0x8015D71C; this overlay passes the selector that
 * ends the work-byte-9 countdown. */
void func_8015D71C(s32 arg0);

/* Drawing routine of this overlay at 0x801EF1DC, still unlifted in its own
 * asm subsegment, which owns the symbol. */
void func_801EF1DC(void);

/* Shared readiness helper of the main executable at 0x8014DAEC. */
void func_8014DAEC(void);

/* @source 0x801EF0E8
 * @behavior Work-byte-9 handler of this overlay's step sequence: reads the
 * scratchpad work-object pointer cell at 0x1F800044 once before the 0x18-byte
 * frame is created and loads the work object's byte 0x9 through that pointer.
 * When the loaded byte equals 0x10 it invokes the main-executable helper at
 * 0x8015D71C with the selector 0x100, re-reads the pointer cell and increments
 * work byte 0x2, advancing the dispatch state. Otherwise it stores the loaded
 * byte plus four back into work byte 0x9 through the pointer loaded at entry.
 * Both paths then call this overlay's drawing routine at 0x801EF1DC and the
 * shared helper at 0x8014DAEC, and return. The frame only keeps $ra across
 * those three calls; nothing is returned and no other state is read or
 * written.
 * @status exact
 * @match 100.00
 * @residual none
 */
void invokeHelperAt100AndAdvanceWorkByte2WhenWorkByte9Is10ElseAdd4ToWorkByte9ThenDrawAndRunHelperBmagicMagic16103_801EF0E8(void) {
  u8 *work;
  u8 byte9;

  work = g_battle_work;
  byte9 = work[9];
  if (byte9 == 0x10) {
    func_8015D71C(0x100);
    g_battle_work[2]++;
  } else {
    work[9] = byte9 + 4;
  }
  func_801EF1DC();
  func_8014DAEC();
}
