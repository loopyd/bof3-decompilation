#include "bof3/bof3.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Byte table at 0x801F0488, inside this payload's tail data block from
 * 0x801F0408 to the end of the image. The section this body reads is 32 bytes
 * of byte values (0x0C, 0x0E and 0x10 alternating in pairs of two), and this
 * function is its only in-payload reader: no other disassembled span of this
 * overlay names the address, so the generated undefined-symbol table does not
 * bind it and the row this lift carries is the target map's. */
extern u8 D_801F0488[]; /* @source 0x801F0488 @kind table */

/* @source 0x801EF2CC
 * @behavior Entry 4 of the five-word handler run at 0x801EEC10, whose word at
 *           0x801EEC14 holds this address and which the dispatcher at
 *           0x801EEEE4 copies and indexes by the work object's byte 2. It
 *           reads the scratchpad work-object pointer cell at 0x1F800044 three
 *           times, once per store group, because every byte store through the
 *           loaded pointer may alias the cell. The first read adds two modulo
 *           256 to the object's unsigned byte 9 with a byte-width
 *           read-modify-write (lbu / addiu 2 / sb); the second adds two to
 *           unsigned byte 0xA the same way; the third takes the object's byte 4
 *           as an index, loads the unsigned byte the table at 0x801F0488 holds
 *           at that index (lui $at,0x801F / addu $at,$at,index / lbu), and
 *           compares the object's byte 9 read from that same pointer with it
 *           (bne). Only on equality does it increment the object's byte 2 from
 *           the pointer the third read already holds, which moves the
 *           byte-2 dispatcher to its next step. The test re-reads byte 9 rather
 *           than reusing the value stored two statements earlier, and no
 *           closure of the two counters depends on that equality, so both
 *           increments are unconditional. The function is a frameless leaf; it
 *           takes no arguments, returns nothing and touches no other state. The
 *           statement order, operand forms, offset spelling and reload points
 *           follow the exact sibling
 *           add2ToWorkBytes9AndAThenAdvanceWorkByte2At16BmagicMagic00903_801EF2D4
 *           and the in-target siblings at 0x801EF348 and 0x801EF394, whose
 *           bodies carry the same three-read frame; this body differs at the
 *           test alone, comparing byte 9 with a table byte instead of the
 *           literal 0x10, so the table load and its index word are written from
 *           this payload's own bytes. Clone was ineligible: a 124-byte scan of
 *           this span over every extracted payload under out/binaries found
 *           only this span's own offset, both as exact bytes and with the
 *           opcode and register fields compared and every immediate masked.
 * @status exact
 * @match 100.00
 * @residual none
 */
void workMatchesTableEntry(void) {
  g_battle_work[0x9] += 2;
  g_battle_work[0xA] += 2;
  if (g_battle_work[0x9] == D_801F0488[g_battle_work[4]]) {
    g_battle_work[2]++;
  }
}
