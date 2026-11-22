#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell. The original re-materializes the cell
 * address (lui $v0,0x1F80 / lw $v0,0x44) once per counter group and once for
 * each of the countdown tail's two groups, so the cell is reached through this
 * address-derived symbol rather than through one cached pointer. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind data */

/* The three counter bytes this handler advances, at the start of the 12-byte
 * records selected by work byte 0xA. The original reaches the second and third
 * counters through their own addresses (lui $at,%hi / addu $at,$at,$v1 before
 * both the load and the store of each pair), so each byte keeps its own array.
 * The bytes at 0x80145BD6 and 0x80145BD8 arrive from this target's generated
 * undefined_syms_auto.txt through the neighbour that still holds those words,
 * and 0x80145BD7 binds from its address-encoded name.
 * @source 0x80145BD6 @kind table
 */
extern u8 D_80145BD6[];
extern u8 D_80145BD7[];
extern u8 D_80145BD8[];

/* @source 0x801EF3C0
 * @behavior Per-frame handler of this overlay's byte-2 handler run: it reads
 *           the scratchpad work-object pointer cell at 0x1F800044 once per
 *           counter group and, for each of the record's three counter bytes,
 *           re-reads the work object's unsigned byte 0xA, scales it by the
 *           12-byte record stride (sll 1, addu, sll 2) into a byte offset and
 *           increments the selected 0x80145BD6 record's bytes 0x0/0x1/0x2 with
 *           a byte-width read-modify-write (lbu, addiu $v0,$v0,1, sb). It then
 *           re-reads the cell, and the work object's unsigned byte 9 with it,
 *           and decrements that byte with the same byte-width form before
 *           testing the stored value against zero (andi $v0,$v0,0xFF, bnez).
 *           Only when the countdown reaches zero it re-reads the cell once more
 *           and increments the object's byte 2 with a byte-width add, moving
 *           the dispatcher on work byte 2 to its next handler step. The second
 *           and third reads of the cell are required: a store through the
 *           loaded work pointer may alias the cell. Leaf with no saved
 *           registers and no calls; takes no arguments, returns nothing and
 *           reads or writes no other state.
 *
 *           The original allocates a 0x10-byte frame whose bytes this body never
 *           touches, with the reservation scheduled into the bnez delay slot and
 *           the release before the return. The declared local below reproduces
 *           exactly that reserved size and takes no use (orphan declared-local
 *           lever); the bytes constrain only the size, so its name and type are a
 *           reconstruction choice. The seed at 0x801EF3D4 recorded 0x108 bytes
 *           and no frame when it dropped that local.
 *
 *           Clone seed: these 0x110 bytes are byte-identical to the span at
 *           0x801EF3D4 of emi/bmagic/magic073/03, whose own lift recorded an
 *           exact match; the equality was proved by a payload cmp of the two
 *           spans before this file was written. The two bodies share every
 *           word, including the frame reservation, because both touch only
 *           main-executable cells and this body has no own-payload reference.
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementWorkByteARecordCountersThenDecrementWorkByte9AndAdvanceWorkByte2OnZeroBmagicMagic07403_801EF3C0(void) {
  /* Reserved frame only; no access and no use. Reconstruction choice. */
  u8 scratch[0x10];
  u32 offset;

  offset = D_1F800044[0xA] * 12;
  D_80145BD6[offset]++;
  offset = D_1F800044[0xA] * 12;
  D_80145BD7[offset]++;
  offset = D_1F800044[0xA] * 12;
  D_80145BD8[offset]++;

  if (--D_1F800044[9] == 0) {
    D_1F800044[2]++;
  }
}
