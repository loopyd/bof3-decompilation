#include "bof3/bof3.h"

/* Scratchpad work-record cursor cell 0x1F800044, bound by this target's own map
 * row. Naming the cell keeps the original's per-read address folding: written
 * as a constant-address dereference the build materializes 0x1F800044 once into
 * a callee-saved register and grows the frame, while the named cell emits the
 * original's lui %hi plus offset lw at every read. */
extern u8* D_1F800044;

/* Shared main-executable context seed this overlay's resource table is keyed
 * by; its generated auto-symbol row covers the address. */
extern u16 D_80143F10;

/* Local resource table at 0x801F5900: four-byte rows of a 16-bit key followed
 * by the row's resource id, so the companion byte sits at 0x801F5902. Both
 * words are interior to the reviewed pointer run labelled D_801F58DC and keep
 * their generated auto-symbol rows. */
extern u16 D_801F5900[];
extern u8 D_801F5902[];

/* Shared resident resource request at 0x8014D6B8: takes the resource id and
 * consumes no result. */
void func_8014D6B8(u32 resource);

/* @source 0x801F3C50
 * @behavior Overlay step-1 handler, entry 1 of the step table at 0x801F61A0
 *           that the overlay update indexes with work-record byte 0x01.
 *           Snapshots work byte 0x0B into byte 0x07 and then acts on that mode
 *           through fresh reads of the record published at the scratchpad
 *           pointer cell 0x1F800044. Mode 1 walks the four-byte rows of the
 *           local table at 0x801F5900 until a row's key halfword equals the
 *           shared seed 0x80143F10, publishes that key as a word at record
 *           offset 0x18 and takes the resource id from the companion byte at
 *           0x801F5902 for the same row. Modes 2, 3 and 4 take resource ids 3,
 *           0 and 1, and any other mode returns without a request. Every
 *           requesting path clears the record word at 0x40, stores 0x10000 as
 *           a word at 0x44, stores 2 to byte 0x48 and 8 to byte 0x09, requests
 *           that resource through the shared helper 0x8014D6B8, and finally
 *           stores the state 2 to record byte 0x01. Takes no arguments, uses no
 *           result and saves only the return address and one callee-saved
 *           register.
 * @status partial
 * @match 82.28
 * @residual 65 of 79 aligned instructions match and the body is 316 bytes
 *           versus the original 312. The first difference is the seed-search
 *           cursor initialization. Advancing offset before the key test keeps
 *           the current row alive separately, but the build exchanges their
 *           registers, moves the row copy to the back edge and increments on
 *           the successful iteration too. The original increments only after
 *           an unequal key. The 0x10000 resource constant and state 2 also
 *           differ in placement around the search exit and initialization
 *           blocks. The third work-record read keeps the literal-address
 *           pointer slot rather than the named cell because the named cell
 *           is commoned with the mode test, dropping four instructions.
 */
void snapshotWorkByteBThenRequestResourceForSeedKeyOrModeAndArmState2World02Area08813_801F3C50(
    void) {
  u8* work;
  u32 offset;
  u32 row;
  u32 value;
  u32 state;

  work = D_1F800044;
  work[0x07] = work[0x0B];

  if (D_1F800044[0x0B] == 1) {
    value = D_80143F10;
    offset = 0;
  search:
    row = offset;
    offset = row + 4;
    if (value == FIELD_REF(u16, D_801F5900, row)) {
      goto found;
    }
    goto search;
  } else {
    work = SPAD_PTR_SLOT(u8, 0x44);
    value = work[0x0B];
    if (value == 2) {
      value = 3;
      goto arm;
    } else if (value == 3) {
      value = 0;
      goto arm;
    } else {
      goto mode_four;
    }
  }
found:
  work = D_1F800044;
  state = 2;
  FIELD_REF(u32, work, 0x18) = value;
  FIELD_REF(u32, work, 0x40) = 0;
  FIELD_REF(u32, work, 0x44) = 0x10000;
  work[0x48] = state;
  D_1F800044[0x09] = 8;
  value = FIELD_REF(u8, D_801F5902, row);
  goto request;
mode_four:
  if (value != 4) {
    return;
  }
  value = 1;
arm:
  state = 2;
  FIELD_REF(u32, work, 0x40) = 0;
  FIELD_REF(u32, work, 0x44) = 0x10000;
  work[0x48] = state;
  D_1F800044[0x09] = 8;
request:
  func_8014D6B8(value);
  D_1F800044[0x01] = state;
}
